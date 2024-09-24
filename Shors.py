from math import gcd, log2, ceil
import numpy as np
import random
import cudaq
from cudaq import *
import fractions
import matplotlib.pyplot as plt
import contfrac

import time # User for timer

# Function to start the timer (in nanoseconds)
def start_timer(title):
    start_time = time.time_ns()  # Get the current time in nanoseconds
    print(f'{title} starting')
    return start_time

# Function to end the timer (in nanoseconds)
def end_timer(title, start_time):
    end_time = time.time_ns()  # Get the current time in nanoseconds
    time_diff = end_time - start_time  # Calculate the total time difference in nanoseconds

    # Convert to seconds, milliseconds, and nanoseconds
    total_seconds = time_diff // 1_000_000_000  # Get the full seconds
    remainder_ns = time_diff % 1_000_000_000  # Get the remaining nanoseconds after seconds
    total_milliseconds = remainder_ns // 1_000_000  # Get the full milliseconds from the remainder
    total_nanoseconds = remainder_ns % 1_000_000  # Get the remaining nanoseconds

    # Print the result in components
    print(f'{title} finished in {total_seconds} s, {total_milliseconds} ms, {total_nanoseconds} ns')


def shors_algorithm(N, initial, quantum):
    """ Factor N using Shor's algorithm with initial starting value and choice of 
    using either classical or quantum approach for the period finding step
    Parameters
    ----------
    N: int 
        Integer to be factored (assumed to be non-prime and >1)
    initial: int 
        Initial choice of the random integer between 2 and N-1
    quantum: boolean
        True if we will call the quantum order-finding algorithm, and false if we call the classical one for step 3.   
        
    Returns
    -------
    a1, a2: int
        Non-trivial factors of N
    """

    # 0. Check to see if N is even.
    if N % 2 == 0:
        divisor1 = 2
        divisor2 = N // 2
        print("Found factors:", divisor1, divisor2)
        return (divisor1, divisor2)

    attempts = [initial]
    while (True):
        # 1. Select a random integer between 2 and N-1
        if len(attempts) == 1:
            a = initial
        else:
            a = random.choice(
                [n for n in range(N - 1) if n not in attempts and n != 1])

        # 2. See if the integer selected in step 1 happens to factor N
        print("Trying a =", a)
        divisor1 = gcd(a, N)
        if divisor1 != 1:
            divisor2 = N // divisor1
            print("Found factors of N={} by chance: {} and {}".format(N, divisor1, divisor2))
            return (divisor1, divisor2)

        # 3. Find the order of a mod N (i.e., r, where a^r = 1 (mod N))
        if quantum == True:
            r = find_order_quantum(a, N)
        else:
            r = find_order_classical(a, N)
        print("The order of a = {} is {}".format(a,r))

        # 4. If the order of a is found and it is
        # * even and
        # * not a^(r/2) = -1 (mod N),
        # then test a^(r/2)-1 and a^(r/2)+1 to see if they share factors with N.
        # We also want to rule out the case of finding the trivial factors: 1 and N.
        divisor1, divisor2 = test_order(a, r, N)
        if (divisor1 != 0):  # test_order will return a 0 if no factor is found
            print("Found factors of N = {}: {} and {}".format(N,divisor1, divisor2))
            return divisor1, divisor2

        # 5. Repeat
        print("retrying...")
        attempts.append(a)


def test_order(a, r, N):
    """Checks whether or not a^(r/2)+1 or a^(r/2)-1 share a non-trivial common factor with N
    Parameters
    ----------
    a: int
    r: int
    N: int
    
    Returns
    -------
    int, int factors of N, if found; 0,0 otherwise  
    """

    if r != None and (r % 2 == 0) and a**r % N == 1:
        if (a**(int(r / 2))) % N != -1:
            possible_factors = [gcd(r - 1, N), gcd(r + 1, N)]
            for test_factor in possible_factors:
                if test_factor != 1 and test_factor != N:
                    return test_factor, N // test_factor
    # period did not produce a factor
    else:
        print('No non-trivial factor found')
        return 0, 0


def find_order_classical(a, N):
    """A naive classical method to find the order r of a (mod N).
    Parameters
    ----------
    a: int
        an integer in the interval [2,N-1]
    N: int
    
    Returns
    -------
    r: int 
        Period of a^x (mod N)
    """
    assert 1 < a and a < N
    r = 1
    y = a
    while y != 1:
        y = y * a % N
        r += 1
    return r


### TODO: Get actuals (classical)
### TESTING FUNCTIONS ###
my_integer = 123  #edit this value to try out a few examples
# Edit the value in the line below to try out different initial guesses for a.
# What happens when you choose a = 42 for the integer 123?
# What happens when you choose a = 100 for the integer 123?
initial_value_to_start = 42  # edit this value; it should be less than my_integer

title = "### Shor's Classical"
start_time = start_timer(title)
shors_algorithm(my_integer, initial_value_to_start, False)
end_timer(title, start_time)
### END TESTING FUNCTIONS ###


# Define kernels for the quantum Fourier transform and the inverse quantum Fourier transform
@cudaq.kernel
def quantum_fourier_transform(qubits: cudaq.qview):
    qubit_count = len(qubits)
    # Apply Hadamard gates and controlled rotation gates.
    for i in range(qubit_count):
        h(qubits[i])
        for j in range(i + 1, qubit_count):
            angle = (2 * np.pi) / (2**(j - i + 1))
            cr1(angle, [qubits[j]], qubits[i])


@cudaq.kernel
def inverse_qft(qubits: cudaq.qview):
    cudaq.adjoint(quantum_fourier_transform, qubits)


### TODO: Generalize these functions
@cudaq.kernel
def modular_mult_5_21(work: cudaq.qview):
    """"Kernel for multiplying by 5 mod 21
    based off of the circuit diagram in 
    https://physlab.org/wp-content/uploads/2023/05/Shor_s_Algorithm_23100113_Fin.pdf
    Modifications were made to change the ordering of the qubits"""
    x(work[0])
    x(work[2])
    x(work[4])
    
    swap(work[0], work[4])
    swap(work[0], work[2])

@cudaq.kernel
def modular_exp_5_21(exponent: cudaq.qview, work: cudaq.qview,
                     control_size: int):
    """ Controlled modular exponentiation kernel used in Shor's algorithm
    |x> U^x |y> = |x> |(5^x)y mod 21>
    """
    x(work[0])
    for exp in range(control_size):
        ctrl_qubit = exponent[exp]
        for _ in range(2**(exp)):
            cudaq.control(modular_mult_5_21, ctrl_qubit, work)
### ENDTODO: Not yet finished

### Specific Demonstration Start ###
# Demonstrate iterated application of 5y mod 21 where y = 1
@cudaq.kernel
def demonstrate_mod_exponentiation(iterations: int):
    qubits = cudaq.qvector(5)
    x(qubits[0]) # initalizes the qubits in the state for y = 1 which is |10000>
    for _ in range(iterations):
        modular_mult_5_21(qubits)


shots = 200

# The iterations variable determines the exponent in 5^x mod 21. 
# Change this value to verify that the demonstrate_mod_exponentiation
# kernel carries out the desired calculation.
iterations = 1  

print(cudaq.draw(demonstrate_mod_exponentiation, iterations))

results = cudaq.sample(demonstrate_mod_exponentiation,
                       iterations,
                       shots_count=shots)

print("Measurement results from sampling:", results)

# Reverse the order of the most probable measured bit string
# and convert the binary string to an integer
integer_result = int(results.most_probable()[::-1],2)

print("For x = {}, 5^x mod 21 = {}".format(iterations, (5**iterations) % 21))
print("For x = {}, the computed result of the circuit is {}".format(
    iterations, integer_result))
### Specific Demonstration End ###

### TODO: Review this code to see how it is optimized for less 
#         qubits and gates than the previous example
@cudaq.kernel
def modular_exp_4_21(exponent: cudaq.qview, work: cudaq.qview):
    """ Controlled modular exponentiation kernel used in Shor's algorithm
     |x> U^x |y> = |x> |(4^x)y mod 21>
     based off of the circuit diagram in https://arxiv.org/abs/2103.13855
     Modifications were made to account for qubit ordering differences"""
    swap(exponent[0], exponent[2])
    # x = 1
    x.ctrl(exponent[2], work[1])

    # x = 2
    x.ctrl(exponent[1], work[1])
    x.ctrl(work[1], work[0])
    x.ctrl([exponent[1], work[0]], work[1])
    x.ctrl(work[1], work[0])

    # x = 4
    x(work[1])
    x.ctrl([exponent[0], work[1]], work[0])
    x(work[1])
    x.ctrl(work[1], work[0])
    x.ctrl([exponent[0], work[0]], work[1])
    x.ctrl(work[1], work[0])
    swap(exponent[0], exponent[2])
### ENDTODO: Not yet finished

### Definining Full Circuit Builder for Specific Example
@cudaq.kernel
def phase_kernel(control_register_size: int, work_register_size: int, a: int,
                 N: int):
    """ 
    Kernel to estimate the phase of the modular multiplication gate |x> U |y> = |x> |a*y mod 21> for a = 4 or 5
    """

    qubits = cudaq.qvector(control_register_size + work_register_size)
    control_register = qubits[0:control_register_size]
    work_register = qubits[control_register_size:control_register_size +
                           work_register_size]

    h(control_register)

    if a == 4 and N == 21:
        modular_exp_4_21(control_register, work_register)
    if a == 5 and N == 21:
        modular_exp_5_21(control_register, work_register, control_register_size)

    inverse_qft(control_register)

    # Measure only the control_register and not the work_register
    mz(control_register)


### Build Circuit for Specific Example
control_register_size = 3
work_register_size = 5
values_for_a = [4, 5]
idx = 1  # change to 1 to select 5
N = 21
shots = 15000

print(
    cudaq.draw(phase_kernel, control_register_size, work_register_size,
               values_for_a[idx], N))

results = cudaq.sample(phase_kernel,
                       control_register_size,
                       work_register_size,
                       values_for_a[idx],
                       N,
                       shots_count=shots)
print(
    "Measurement results for a={} and N={} with {} qubits in the control register "
    .format(values_for_a[idx], N, control_register_size))
print(results)

### Reviewing Our Results
def top_results(sample_results, zeros, threshold):
    """Function to output the non-zero results whose counts are above the given threshold
    Returns
    -------
        dict[str, int]: keys are bit-strings and values are the respective counts  
    """
    results_dictionary = {k: v for k, v in sample_results.items()}
    if zeros in results_dictionary.keys():
        results_dictionary.pop(zeros)
    sorted_results = {
        k: v for k, v in sorted(
            results_dictionary.items(), key=lambda item: item[1], reverse=True)
    }
    top_key = next(iter(sorted_results))
    max_value = sorted_results[top_key]
    top_results_dictionary = {top_key: max_value}

    for key in sorted_results:
        if results_dictionary[key] > min(threshold, max_value):
            top_results_dictionary[key] = results_dictionary[key]
    return top_results_dictionary
top_results(results, '000', 750)


def get_order_from_phase(phase, phase_nbits, a, N):
    """Uses continued fractions to find the order of a mod N  
    Parameters
    ----------
    phase: int
        Integer result from the phase estimate of U|x> = ax mod N
    phase_nbits: int
        Number of qubits used to estimate the phase
    a: int
        For this demonstration a is either 4 or 5
    N: int
        For this demonstration N = 21
    Returns
    -------
    int: period of a mod N if found, otherwise returns None
    """

    assert phase_nbits > 0
    assert a > 0
    assert N > 0

    eigenphase = float(phase) / 2**(phase_nbits)

    f = fractions.Fraction.from_float(eigenphase).limit_denominator(N)

    if f.numerator == 1:
        return None
    eigenphase = float(f.numerator / f.denominator)
    print('eigenphase is ', eigenphase)
    coefficients_continued_fraction = list(
        contfrac.continued_fraction(eigenphase))
    
    convergents_continued_fraction = list(contfrac.convergents(eigenphase))
    print('convergent sequence of fractions for this eigenphase is', convergents_continued_fraction)
    for r in convergents_continued_fraction:
        print('using the denominators of the fractions in the convergent sequence, testing order =', r[1])
        if a**r[1] % N == 1:
            print('Found order:', r[1])
            return (r[1])
    return None

### TODO: Update this to work for any a & N
def find_order_quantum(a, N):
    """The quantum algorithm to find the order of a mod N, when x = 4 or x =5 and N = 21
    Parameters
    ----------
    a: int
        For this demonstration a will be either 4 or 5
    N: int
        For this demonstration N will be 21
    Returns
    r: int the period if it is found, or None if no period is found
    -------
    """

    if (a == 4 and N == 21) or (a == 5 and N == 21):
        shots = 15000
        if a == 4 and N == 21:
            control_register_size = 3
            work_register_size = 2
        if a == 5 and N == 21:
            control_register_size = 5
            work_register_size = 5

        #cudaq.set_random_seed(123)
        results = cudaq.sample(phase_kernel,
                               control_register_size,
                               work_register_size,
                               a,
                               N,
                               shots_count=shots)
        print("Measurement results:")
        print(results)

        # We will want to ignore the all zero result
        zero_result = ''.join(
            [str(elem) for elem in [0] * control_register_size])
        # We'll only consider the top results from the sampling
        threshold = shots * (.1)
        most_probable_bitpatterns = top_results(results, zero_result, threshold)

        for key in most_probable_bitpatterns:
            # Convert the key bit string into an integer 
            # This integer divided by 8 is an estimate for the phase
            reverse_result = key[::-1]
            phase = int(reverse_result, 2)
            
            print("Trying nonzero bitpattern from the phase estimation:", key,
                  "=", phase)
            r = get_order_from_phase(phase, control_register_size, a, N)
            if r == None:
                print('No period found.')

                continue

            return r
            break
    else:
        print(
            "A different quantum kernel is required for this choice of a and N."
        )

### Running Final Test
my_integer = 21
initial_value_to_start = 5  # Try replacing 5 with 4
quantum = True
title = "### Shor's Quantum (Simulated)"
start_time = start_timer(title)
shors_algorithm(my_integer, initial_value_to_start, quantum)
end_timer(title, start_time)
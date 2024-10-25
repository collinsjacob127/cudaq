/**
 * Author: Jacob Collins
 * Description: An example implementation of Shor's Algorithm
 * Instructions:
 *   Make:
 *     $ make shors
 *     OR
 *     $ nvq++ shors.cpp -o shors.o
 *   Run: 
 *     ./shors.o
 */

#include <cudaq.h>
#include <iostream> // Terminal output
#include <random> // Get a random number
#include <sstream> //String formatting
#include <string> // Strings are good
#include <vector> // Lists of numbers
#include <algorithm> // sort
#include <chrono> // Timer

#include "fractionizer.h" // Temporary CF solution

/****************** HELPER FUNCS ******************/
// Convert value to binary string
template <typename T>
std::string bin_str(T val, T nbits) {
  std::stringstream ss;
  for (T i = 1; i <= nbits; ++i) {
    // Shift through the bits in val
    auto target_bit_set = (1 << (nbits - i)) & val;
    // Add matching val to string
    if (target_bit_set) {
      ss << '1';
    } else {
      ss << '0';
    }
  }
  return ss.str();
}


// Return max value in an array
template <typename T>
T max(std::vector<T> arr) {
  T max = arr[0];
  for (auto &v : arr) {
    if (v > max) {
      max = v;
    }
  }
  return max;
}

// Convert bin string to long. 1101 -> 13
long bin_to_int(std::string &s) {
  long result = 0;
  int len = s.length();
  for (int i = 0; i < len; ++i) {
    if (s[i] == '1') {
      result += pow(2, len - 1 - i);
    }
  }
  return result;
}

// Convert an array to a string
template <typename T>
std::string arrayToString(std::vector<T> arr) {
  std::stringstream ss;
  // if (binary) {
  //   for (T i = 0; i < arr.size(); i++) {
  //     ss << bin_str(arr[i], nbits);
  //     if (i < arr.size() - 1) {
  //       ss << ", ";
  //       ;
  //     }
  //   }
  // } else {
  ss << arr[0];
  for (T i = 1; i < arr.size(); i++) {
    ss << ", " << arr[i];
  }
  // }
  return ss.str();
}

template <typename T>
std::string to_string(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

// Convert nanoseconds to a string displaying subdivisions of time
std::string format_time(long long nanoseconds) {
    long long milliseconds = nanoseconds / 1'000'000;  // From ns to ms
    long long seconds = milliseconds / 1'000;          // From ms to seconds
    milliseconds = milliseconds % 1'000;               // Remaining milliseconds

    long long remaining_nanoseconds = nanoseconds % 1'000'000; // Remaining nanoseconds
    long long microseconds = remaining_nanoseconds / 1'000;    // From ns to µs
    remaining_nanoseconds = remaining_nanoseconds % 1'000;     // Remaining ns

    // Create the formatted string
    return to_string(seconds) + "s " +
           to_string(milliseconds) + "ms " +
           to_string(microseconds) + "µs " +
           to_string(remaining_nanoseconds) + "ns";
}

/***************************************
*************** QUANTUM ****************
***************************************/

//  Inverse Quantum Fourier Transform
// This is straight from a NVIDIA example program
__qpu__ void iqft(cudaq::qview<> q) {
  int N = q.size();
  // Swap qubits
  for (int i = 0; i < N / 2; ++i) {
    swap(q[i], q[N - i - 1]);
  }

  for (int i = 0; i < N - 1; ++i) {
    h(q[i]);
    int j = i + 1;
    for (int y = i; y >= 0; --y) {
      double denom = (1UL << (j - y));
      const double theta = -M_PI / denom;
      r1<cudaq::ctrl>(theta, q[j], q[y]);
    }
  }

  h(q[N - 1]);
}

// "Modular" mult - Copied from Nvidia shors.ipynb
struct modular_mult_5_21 {
  void operator()(cudaq::qview<> work) __qpu__ {
    /* Kernel for multiplying by 5 mod 21
    * based off of the circuit diagram in
    * https://physlab.org/wp-content/uploads/2023/05/Shor_s_Algorithm_23100113_Fin.pdf
    * Modifications were made to change the ordering of the qubits
    */
    x(work[0]);
    x(work[2]);
    x(work[4]);

    swap(work[0], work[4]);
    swap(work[0], work[2]);
  }
};

// "Modular" Exp - Copied from Nvidia shors.ipynb
__qpu__ void modular_exp_5_21(cudaq::qview<> exp, cudaq::qview<> work, const int control_size) {
  /* Controlled modular exponentiation kernel used in Shor's algorithm
   * |x> U^x |y> = |x> |(5^x)y mod 21>
   */
  x(work[0]);
  for (int i = 0; i < control_size; ++i) {
    for (int j = 0; j < pow(2, i); ++j) {
      // https://nvidia.github.io/cuda-quantum/latest/specification/cudaq/synthesis.html
      cudaq::control(modular_mult_5_21{}, exp[i], work);
    }
  }
}

struct demo_mod_exp {
  void operator()(const int max_iter) __qpu__ {
    auto qubits = cudaq::qvector(5);
    auto mod_multer = modular_mult_5_21{};
    x(qubits[0]);
    for (int i = 0; i < max_iter; ++i) {
      mod_multer(qubits);
    }
  }
};

void run_mod_exp_demo(int shots, int iterations) {
  // Generate and run demo circuit
  printf("\nRunning quantum mod exponentiation demo (x=%d)\n", iterations);
  auto start = std::chrono::high_resolution_clock::now();

  auto counts = cudaq::sample(shots, demo_mod_exp{}, iterations);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Mod Exp Demo finished in %s.\n", format_time(duration).c_str());

  // Print expected val
  printf("For x = %d, 5^x mod 21 = %ld\n", 
          iterations, 
          ((long) pow(5, iterations) % 21));

  // Reverse bit order of most probable measured bit
  std::string result = counts.most_probable();
  std::reverse(result.begin(), result.end()); 
  // Print computed val
  printf("For x = %d, computed result from demo is: %ld\n",
          iterations, bin_to_int(result));
  return;
}

__qpu__ void modular_exp_4_21(cudaq::qview<> exp, cudaq::qview<> work) {
  swap(exp[0], exp[2]);
  // x = 1
  x<cudaq::ctrl>(exp[2], work[1]);

  // x = 2
  x<cudaq::ctrl>(exp[1], work[1]);
  x<cudaq::ctrl>(work[1], work[0]);
  x<cudaq::ctrl>(exp[1], work[0], work[1]);
  x<cudaq::ctrl>(work[1], work[0]);

  // x = 4
  x(work[1]);
  x<cudaq::ctrl>(exp[0], work[1], work[0]);
  x(work[1]);
  x<cudaq::ctrl>(work[1], work[0]);
  x<cudaq::ctrl>(exp[0], work[0], work[1]);
  x<cudaq::ctrl>(work[1], work[0]);
  swap(exp[0], exp[2]);
}

/**
 * @brief Kernel to estimate the phase of the modular multiplication gate.
 *        |x> U |y> = |x> |a*y mod 21> for a = 4 or 5
 * @param nbits_ctrl The number of bits to allocate to the control register.
 * @param nbits_work The number of bits to allocate to the work register.
 * @param a The value for which we are now calculating periodicity.
 * @param n The value we are trying to factor.
 */
struct phase_kernel {
  __qpu__ auto operator()(const long nbits_ctrl, 
                          const long nbits_work,
                          const long a,
                          const long n) {
    // Initialize registers
    cudaq::qvector qs(nbits_ctrl + nbits_work);
    auto ctrl_reg = qs.front(nbits_ctrl);
    auto work_reg = qs.back(nbits_work);

    // Run ops
    h(ctrl_reg);
    if (a == 4 && n == 21) {
      modular_exp_4_21(ctrl_reg, work_reg);
    }
    if (a == 5 && n == 21) {
      modular_exp_5_21(ctrl_reg, work_reg, nbits_ctrl);
    }
    iqft(ctrl_reg);
    mz(ctrl_reg);
  }
};

void test_phase_kernel(long nbits_ctrl, 
                       long nbits_work, 
                       std::vector<long> a_vals, 
                       uint idx, 
                       long n, 
                       int shots) {
  // Validate inputs
  if (a_vals[idx] != 4 && a_vals[idx] != 5) {
    printf("Invalid 'a' val to phase kernel test\n");
    return;
  }
  if (n != 21) {
    printf("Invalid 'n' to phase kernel test\n");
  }

  // Test phase kernel
  printf("\nTesting phase kernel...\n");
  auto start = std::chrono::high_resolution_clock::now();
  auto counts = cudaq::sample(shots, phase_kernel{}, nbits_ctrl, nbits_work, a_vals[idx], n);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Phase Kernel Test finished in %s.\n", format_time(duration).c_str());

  printf("Measurement results for a=%ld and n=%ld with %ld qubits in ctrl register:\n", a_vals[idx], n, nbits_ctrl);
  // Reverse bit order of most probable measured bit
  std::string result = counts.most_probable();
  std::reverse(result.begin(), result.end()); 
  // Print computed val
  printf("  %ld (%s)\n",
          bin_to_int(result), result.c_str());

}

/**
 * @brief Find the order of a mod N. Needs to be updated to
 *        use continued fractions (CF).
 * @param phase Integer result from the phase estimate of U|x> = ax mod N
 * @param nbits Number of qubits used to estimate the phase.
 * @param a Either 4 or 5, in this demo
 * @param N The number being factored. In this demo it's 21.
 * @return Integer period of a mod N if found, otherwise -1.
 */
int get_order_from_phase(int phase, int nbits, int a, int n) {
  if (nbits <= 0 || a <= 0 || n <= 0) {
    // invalid input
    return -1;
  }

  long double num, denom;
  long double eigenphase = (long double) phase / (long double) pow(2, nbits);
  auto seq = Fractionizer::fractionize(eigenphase, num, denom);
  if (num == (long double) 1) {
    printf("Numerator was initially 1. Exiting order from phase.\n");
    return -1;
  }
  eigenphase = num / denom;
  printf("Eigenphase is %Lf\n", eigenphase);
  printf("Sequence: %s", arrayToString(seq).c_str());
  for (auto &r : seq) {
    printf("Using denoms of fractions in convergent sequence, testing order = %Lf", r);
    if ((int) pow(a, r) % n == 1) {
      printf("Found order: %Lf\n", r);
      return r;
    }
  }
  return -1;
}

/**
 * @brief The quantum algorithm to find the order of a mod N, when x = 4 or x = 5 and N = 21.
 * @param a Integer, 4 or 5 
 * @param n Number to be factored; 21.
 * @return Int the period if it is found, or -1 if no period is found.
 */
long find_order_quantum(long a, long n) {
  if ((a != 4 && a != 5) || n != 21) {
    return -1;
  }
  int shots = 15000;
  int nbits_ctrl, nbits_work;
  if (a == 4) {
    nbits_ctrl = 3;
    nbits_work = 2;
  }
  if (a == 5) {
    nbits_ctrl = 5;
    nbits_work = 5;
  }
  auto counts = cudaq::sample(shots, phase_kernel{}, nbits_ctrl, nbits_work, a, n);
  counts.dump();

  //TODO: Filter output values and find most probable VALID output
  return -1;
}

/*****************************************
*************** CLASSICAL ****************
*****************************************/

// 
long find_order_classical(long a, long n) {
  if (a <= 1 || a >= n) {
    printf("Bad input to find_order_classical\n");
    return -1;
  }
  long r = 1;
  long y = a;
  while (y != 1) {
    // printf("y: %ld a: %ld, n: %ld, y * a %% n: %ld\n", y, a, n, y * a % n);
    y = y * a % n;
    r++;
  }
  return r;
}

// Select a random integer between 2 and n
// Selected integer cannot be in attempts
long select_a(long n, std::vector<long> attempts) {
  if (n < 3) {
    printf("Unacceptable selection of a\n");
    return -1;
  }
  // Generate max range
  std::vector<long> poss_vals;
  for (long i = 0; i < n; ++i) {
    poss_vals.push_back(i);
  }
  // Remove attempted vals
  std::sort(attempts.begin(), attempts.end());
  int n_erased = 0;
  for (const auto &x : attempts) {
    poss_vals.erase(poss_vals.begin() + x - n_erased);
    n_erased++;
  }
  poss_vals.erase(poss_vals.begin()); // Remove 0
  poss_vals.erase(poss_vals.begin()); // Remove 1
  
  // Select random val from array
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(0, poss_vals.size()-1);
  int rand_index = distr(gen);
  return poss_vals[rand_index];
}

// Euclid's alg
long gcd(long a, long b) {
  while (b != 0) {
    long temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

// Check whether or not a^(r/2)+1 or a^(r/2)-1 share a non-trivial
// common factor with N
std::vector<long> test_order(long a, long r, long n) {
  if ((long) pow(a, r/2) % n != -1) {
    std::vector<long> poss_factors = {gcd(r-1, n), gcd(r+1, n)};
    for (auto &test_factor : poss_factors) {
      if (test_factor != 1 && test_factor != n) {
        return {test_factor, n / test_factor};
      }
    }
  }
  return {0, 0};
}

/**************************************
*************** DRIVER ****************
**************************************/

std::vector<long> shors(long n, long initial, bool quantum) {
  // Handle even numbers
  if (n % 2 == 0) {
    return {2, n / 2};
  }
  
  std::vector<long> attempts = {initial};
  uint max_iter = 10000;
  long a = initial;
  long divisor1; long divisor2;
  while (attempts.size() < max_iter) {
    // 1. Select random integer between 2 and N-1
    if (attempts.size() != 1) {
      a = select_a(n, attempts);
    }

    // 2. Check if selected integer factors N
    divisor1 = gcd(a, n);
    if (divisor1 != 1) {
      divisor2 = n / divisor1;
      return {divisor1, divisor2, (long) attempts.size()};
    }

    // 3. Find the order of a mod N (i.e., r, where a^r = 1 (mod N))
    long r;
    if (quantum) {
      r = find_order_quantum(a, n);
    } else {
      r = find_order_classical(a, n);
    }

    // 4. If the order of a is found and it is
    // even and not a^(r/2) = -1 (mod N),
    // then test a^(r/2)-1 and a^(r/2)+1 to see if they share factors with N.
    // We also want to rule out the case of finding the trivial factors: 1 and N.
    std::vector<long> divisors = test_order(a, r, n);
    divisor1 = divisors[0]; divisor2 = divisors[1];
    if (divisor1 != 0) { // test_order returns {0, 0} if no factor found
      return {divisor1, divisor2, (long) attempts.size()};
    }
    attempts.push_back(a);
  }
  return {-1, -1, (long) attempts.size()};
}

int main(int argc, char *argv[]) {
  long fact1 = 11;
  long fact2 = 23;
  if (argc >= 3) {
    fact1 = strtol(argv[1], nullptr, 10);
    fact2 = strtol(argv[2], nullptr, 10);
  }
  // Set up params
  long initial_val = 4;
  bool quantum = false;

  printf("Inputs:\n");
  printf("f1 = %ld\nf2 = %ld\nN = %ld\n",fact1, fact2, fact1*fact2);
  if (quantum) {
    printf("\n(Quantum Implementation)\n");
  } else {
    printf("\n(Classical Implementation)\n");
  }
  printf("Running Shor's...\n");

  auto start = std::chrono::high_resolution_clock::now();
  std::vector<long> factors = shors(fact1*fact2, initial_val, quantum);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  printf("Shor's finished in %s.\n", format_time(duration).c_str());
  printf("%ld attempt(s)\n", factors[2]);
  printf("Output:\n");
  if (factors[0] == -1) {
    printf("No factors found\n");
  } else {
    printf("Factor 1: %ld\n", factors[0]);
    printf("Factor 2: %ld\n", factors[1]);
    printf("Product: %ld\n", factors[0]*factors[1]);
  }

  printf("--Demos--\n");
  run_mod_exp_demo(200, 2);
  test_phase_kernel(3, 5, {4, 5}, 1, 21, 15000);
  return 0;
}

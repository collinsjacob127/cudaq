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
#include <fstream> // Save circuit as QASM

/****************** HELPER FUNCS ******************/
// Convert value to binary string
template <typename T>
std::string bin_str(T val, int nbits) {
  std::stringstream ss;
  for (int i = 1; i <= nbits; ++i) {
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
std::string arrayToString(std::vector<T> arr, bool binary, int nbits) {
  std::stringstream ss;
  if (binary) {
    for (int i = 0; i < arr.size(); i++) {
      ss << bin_str(arr[i], nbits);
      if (i < arr.size() - 1) {
        ss << ", ";
        ;
      }
    }
  } else {
    ss << arr[0];
    for (int i = 1; i < arr.size(); i++) {
      ss << ", " << arr[i];
    }
  }
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
    // This needs some tinkering
    auto mod_multer = modular_mult_5_21{};
    x(qubits[0]);
    for (int i = 0; i < max_iter; ++i) {
      // So does this
      mod_multer(qubits);
    }
  }
};

void run_mod_exp_demo(int shots, int iterations) {
  // Draw circuit with one iteration, 200 shots
  
  // Sample said circuit

  // Print results

  // Reverse bit order of most probable measured bit

  // Convert to int and print

  return;
}

// TODO: Phase Kernel

// TODO: Order from phase

// TODO: Find Order

long find_order_quantum(long a, long n) {
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
      return {divisor1, divisor2};
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
      return {divisor1, divisor2};
    }
    attempts.push_back(a);
  }
  return {-1, -1};
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
    printf("(Quantum Implementation)\n");
  } else {
    printf("(Classical Implementation)\n");
  }
  printf("Running Shor's...\n");

  auto start = std::chrono::high_resolution_clock::now();
  std::vector<long> factors = shors(fact1*fact2, initial_val, false);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  printf("Shor's finished in %s.\n", format_time(duration).c_str());
  printf("Output:\n");
  if (factors[0] == -1) {
    printf("No factors found\n");
  } else {
    printf("Factor 1: %ld\n", factors[0]);
    printf("Factor 2: %ld\n", factors[1]);
    printf("Product: %ld\n", factors[0]*factors[1]);
  }
  return 0;
}

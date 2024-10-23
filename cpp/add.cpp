/**********************************
 * Description: Example Adder using CudaQ
 * Author: Jacob Collins
 * Instructions:
 *   Compile and run with:
 *   ```
 *   $> make add
 *   $> ./add # Uses default values
 *   OR
 *   $> ./add 00101 11101 # Takes binary input
 *   ```
 **********************************/
#include <cudaq.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

/****************** HELPER FUNCS ******************/
// Convert some value to a string
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

// Convert bin string to int. 1101 -> 13
int bin_to_int(std::string &s) {
  int result = 0;
  int len = s.length();
  for (int i = 0; i < len; ++i) {
    if (s[i] == '1') {
      result += pow(2, len - 1 - i);
    }
  }
  return result;
}

/****************** CUDAQ FUNCS ******************/

// Apply NOT-gates in accordance with bit-pattern of given integer.
__qpu__ void set_int(const long val, cudaq::qvector<> &qs) {
  // Iterate through bits in val
  for (int i = 1; i <= qs.size(); ++i) {
    // Bit-shift for single bitwise AND to apply X on correct qubits
    auto target_bit_set = (1 << (qs.size() - i)) & val;
    // Apply X if bit i is valid
    if (target_bit_set) {
      x(qs[i - 1]);
    } 
  }
}

// Bitwise addition of v_reg1 and v_reg2.
// Output is {c_reg[0], v_reg2}
__qpu__ void add(cudaq::qvector<> &v_reg1, cudaq::qvector<> &v_reg2,
                 cudaq::qvector<> &c_reg) {
  const int nbits_v = v_reg1.size();

  // Store all the carries in c_reg
  for (int i = nbits_v - 1; i >= 0; --i) {
    x<cudaq::ctrl>(v_reg1[i], v_reg2[i], c_reg[i]);
    x<cudaq::ctrl>(v_reg1[i], v_reg2[i]);
    x<cudaq::ctrl>(c_reg[i + 1], v_reg2[i], c_reg[i]);
  }
  // Update reg 2 highest-order bit
  x<cudaq::ctrl>(v_reg1[0], v_reg2[0]);
  for (int i = 0; i < nbits_v; ++i) {
    // Perform sum with carries, send to reg 2
    x<cudaq::ctrl>(v_reg1[i], v_reg2[i]);
    x<cudaq::ctrl>(c_reg[i + 1], v_reg2[i]);
    if (i < nbits_v - 1) {
      // Undo carries, except highest-order carry bit
      x<cudaq::ctrl>(c_reg[i + 2], v_reg2[i + 1], c_reg[i + 1]);
      x<cudaq::ctrl>(v_reg1[i + 1], v_reg2[i + 1]);
      x<cudaq::ctrl>(v_reg1[i + 1], v_reg2[i + 1], c_reg[i + 1]);
    }
  }
  return;
}

/****************** CUDAQ STRUCTS ******************/
// Driver for adder
struct run_adder {
  __qpu__ auto operator()(const long val1, 
                          const long val2) {
    // 1. Compute Necessary Bits
    // Necessary # bits computed based on input values. Min 1.
    int nbits_val = ceil(log2(max(std::vector<long>({val1, val2, 1})) + 1));
    // Sum register needs 1 extra bit (111 + 111 = 1110)
    int nbits_sum = nbits_val + 1;

    // 2. Initialize Registers
    cudaq::qvector v_reg1(nbits_val);  // Value 1 reg
    cudaq::qvector v_reg2(nbits_val);  // Value 2 reg
    cudaq::qvector c_reg(nbits_sum);   // Sum reg
    set_int(val1, v_reg1);
    set_int(val2, v_reg2);

    // 3. Add
    add(v_reg1, v_reg2, c_reg);
    // 4. Measure
    // Sum is {c_reg[0], v_reg2}
    mz(v_reg1, v_reg2, c_reg);
  }
};

/****************** CUDAQ STRUCTS ******************/
int main(int argc, char *argv[]) {
  // PARSE INPUT VALUES
  // Values to add, optionally passed in cmdline
  long val1 = 0b1000;
  long val2 = 0b0100;
  if (argc >= 3) {
    val1 = strtol(argv[1], nullptr, 2);
    val2 = strtol(argv[2], nullptr, 2);
  }
  // Necessary # bits computed based on input values. Min 1.
  int nbits_val = ceil(log2(max(std::vector<long>({val1, val2, 1})) + 1));

  printf("Adding values: %ld + %ld (%s + %s)\n", val1, val2,
         bin_str(val1, nbits_val).c_str(), bin_str(val2, nbits_val).c_str());

  // GENERATE AND RUN CIRCUIT
  auto start = std::chrono::high_resolution_clock::now();

  auto counts = cudaq::sample(run_adder{}, val1, val2);

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Adder finished in %s.\n", format_time(duration).c_str());
  
  // REVIEW RESULTS
  std::string result = counts.most_probable();
  printf("Full out: (%s)\n", result.c_str());
  std::string val2_out = result.substr(nbits_val, nbits_val);
  std::string sum_out = result.substr(2 * nbits_val, 1) + val2_out;
  printf("Sum: %d (%s)\n", bin_to_int(sum_out), sum_out.c_str());
}

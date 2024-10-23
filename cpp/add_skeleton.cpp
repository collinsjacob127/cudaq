/**********************************
 * Description: Example Adder using CudaQ
 * Author: <Your name here>
 * Instructions:
 *   Compile and run with:
 *   ```
 *   $> make add_skeleton
 *   $> ./add_skeleton # Uses default values
 *   OR
 *   $> ./add_skeleton 00101 11101 # Takes binary input
 *   ```
 *   (You can rename it to whatever you want)
 **********************************/
/**********************************
 * YOU WILL NEED TO LOOK AT THE DIAGRAMS
 * TO KNOW HOW TO IMPLEMENT THE ADDER
 * https://github.com/JAllsop/Quantum-Full-Adder/blob/master/ELEN4022_Lab_2_2021.ipynb
 * Another option that may be more efficient, but requires some changes
 * in the outline of the code:
 * https://tsmatz.wordpress.com/2019/05/22/quantum-computing-modulus-add-subtract-multiply-exponent/
 **********************************/
#include <cudaq.h>

#include <bitset>
#include <cmath>
#include <iostream>
#include <numbers>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

/****************** HELPER FUNCS ******************/
// Convert value to binary string
template <typename T>
std::string bin_str(T v, int nbits) {
  std::stringstream ss;
  ss << std::bitset<sizeof(T) * 8>(v).to_string().substr(sizeof(T) * 8 - nbits,
                                                         nbits);
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

// QPU instructions for adding 2 value registers aided by a carry register
// This will be helpful:
// https://github.com/JAllsop/Quantum-Full-Adder/blob/master/ELEN4022_Lab_2_2021.ipynb
__qpu__ void add(cudaq::qvector<> &v_reg1, cudaq::qvector<> &v_reg2,
                 cudaq::qvector<> &c_reg) {
  const int nbits_v = v_reg1.size();

  // Propogate carried bits
  for (int i = nbits_v - 1; i >= 0; --i) {
    // x<cudaq::ctrl>(ctrl1, ctrl2, tgt); <- something like this
    // TODO: <YOUR CODE HERE>
    // ...
  }
  // Update reg 2 highest-order bit
  // x<cudaq::ctrl>(ctrl, tgt); <- something like this
  // TODO: <YOUR CODE HERE>
  // ...
  for (int i = 0; i < nbits_v; ++i) {
    // Perform sum, send to reg 2 (bit order is tricky here)
    // x<cudaq::ctrl>(ctrl, tgt); <- something like this
    // TODO: <YOUR CODE HERE>
    // ...
    if (i < nbits_v - 1) {
      // Undo carries, except highest-order carry bit
      // (Inverse of carry operation)
      // TODO: <YOUR CODE HERE>
      // ...
    }
  }
  return;
}

/****************** CUDAQ STRUCTS ******************/
// Driver for adder
struct run_adder {
  template <typename CallableKernel>
  __qpu__ auto operator()(const int nbits_val, const int nbits_sum,
                          CallableKernel &&val1_setter,
                          CallableKernel &&val2_setter) {
    // Initialize Registers
    cudaq::qvector v_reg1(nbits_val);  // Value 1 reg
    cudaq::qvector v_reg2(nbits_val);  // Value 2 reg
    cudaq::qvector c_reg(nbits_sum);   // Sum reg

    // Set values
    val1_setter(v_reg1);
    val2_setter(v_reg2);
    // Add
    add(v_reg1, v_reg2, c_reg);
    // Measure
    mz(v_reg1, v_reg2, c_reg);
  }
};

// Apply not-gates matching binary pattern of val
// 7 -> 0111
struct int_setter {
  const long val;
  void operator()(cudaq::qvector<> &qs) __qpu__ {
    // Iterate through bits in val
    for (int i = 1; i <= qs.size(); ++i) {
      // Bit-shift for single bitwise AND to apply X on correct qubits
      // TODO: <YOUR CODE HERE>
      // ...
    }
  }
};

/****************** CUDAQ STRUCTS ******************/
// Nothing needs to be changed here
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
  // Sum register needs 1 extra bit (111 + 111 = 1110)
  int nbits_sum = nbits_val + 1;
  printf("Adding values: %ld + %ld (%s + %s)\n", val1, val2,
         bin_str(val1, nbits_val).c_str(), bin_str(val2, nbits_val).c_str());

  // GENERATE AND RUN CIRCUIT
  int_setter val1_setter{.val = val1};
  int_setter val2_setter{.val = val2};
  auto counts = cudaq::sample(run_adder{}, nbits_val, nbits_sum, val1_setter,
                              val2_setter);

  // REVIEW RESULTS
  std::string result = counts.most_probable();
  std::string val2_out = result.substr(nbits_val, nbits_val);
  std::string sum_out = result.substr(2 * nbits_val, 1) + val2_out;
  printf("Sum: %d (%s)\n", bin_to_int(sum_out), sum_out.c_str());
}

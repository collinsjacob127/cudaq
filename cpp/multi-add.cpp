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
// QPU instructions for adding 2 value registers
// aided by a carry register
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
    
    // std::vector<cudaq::qudit> out_qubits;
    // out_qubits.push_back(c_reg[0]);
    // for (int i = 0; i < nbits_val; ++i) {
    //     out_qubits.push_back(v_reg2[i]);
    // }
    // Measure
    // mz(out_qubits);
    mz(v_reg1, v_reg2, c_reg);
  }
};

// Apply not-gates matching binary pattern of val
struct int_setter {
  const long val;
  void operator()(cudaq::qvector<> &qs) __qpu__ {
    // Iterate through bits in val
    for (int i = 1; i <= qs.size(); ++i) {
      // Bit-shift for single bitwise AND to apply X on correct qubits
      auto target_bit_set = (1 << (qs.size() - i)) & val;
      // Apply X if bit i is valid
      if (target_bit_set) x(qs[i - 1]);
    }
  }
};

/****************** MAIN ******************/
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

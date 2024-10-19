// Example Adder using CudaQ
// Compile and run with:
// ```
// make add 
// ./add
// OR 
// ./add 00101 11101
// ```

// Base includes
#include <cudaq.h>

#include <cmath>
#include <numbers>
// Custom includes
#include <bitset>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
// #include <cstring>

/****************** HELPER FUNCS ******************/
// Convert value to binary string
template <typename T>
std::string bin_str(T v, int nbits) {
  std::stringstream ss;
  ss << std::bitset<sizeof(T) * 8>(v).to_string().substr(
          sizeof(T) * 8 - nbits, nbits);
  return ss.str();
}

// Convert vector to str formatted list of binary strings
template <typename T>
std::string array_to_string(std::vector<T> arr, bool binary, int nbits) {
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

long max(std::vector<long> arr) {
  long max = arr[0];
  for (auto &v : arr) {
    if (v > max) {
      max = v;
    }
  }
  return max;
}

// Convert bin string to int. 1101 -> 13
int bin_to_int(std::string& s) {
  int result = 0;
  int len = s.length();
  for (int i = 0; i < len; ++i) {
    if (s[i] == '1') { result += pow(2, len-1-i); }
  }
  return result;
}

/*********** CudaQ Funcs ************/
// Add values 
__qpu__ void add(cudaq::qvector<> &v_reg_1, 
                 cudaq::qvector<> &v_reg_2, 
                 cudaq::qvector<> &c_reg) {
  const int nbits_v = v_reg_1.size();

  // Carry
  for (int i = nbits_v-1; i >= 0; --i) {
    x<cudaq::ctrl>(v_reg_1[i], v_reg_2[i], c_reg[i]);
    x<cudaq::ctrl>(v_reg_1[i], v_reg_2[i]);
    x<cudaq::ctrl>(c_reg[i+1], v_reg_2[i], c_reg[i]);
  }
  x<cudaq::ctrl>(v_reg_1[0], v_reg_2[0]);
  for (int i = 0; i < nbits_v-1; ++i) {
    // Sum
    x<cudaq::ctrl>(v_reg_1[i], v_reg_2[i]);
    x<cudaq::ctrl>(c_reg[i+1], v_reg_2[i]);
    if (i < nbits_v-1) {
        // Undo Carry
        x<cudaq::ctrl>(c_reg[i+2], v_reg_2[i+1], c_reg[i+1]);
        x<cudaq::ctrl>(v_reg_1[i+1], v_reg_2[i+1]);
        x<cudaq::ctrl>(v_reg_1[i+1], v_reg_2[i+1], c_reg[i+1]);
    }
  }
  // Sum
  x<cudaq::ctrl>(v_reg_1[nbits_v-1], v_reg_2[nbits_v-1]);
  x<cudaq::ctrl>(c_reg[nbits_v], v_reg_2[nbits_v-1]);
  return;
}


/********** CudaQ Structs ************/
struct run_adder {
  template <typename CallableKernel>
  __qpu__ auto operator()(const int nbits_val, 
                          const int nbits_sum,
                          CallableKernel &&val1_setter, 
                          CallableKernel &&val2_setter) {
    cudaq::qvector v_reg_1(nbits_val); // Value 1 reg
    cudaq::qvector v_reg_2(nbits_val); // Value 2 reg
    cudaq::qvector c_reg(nbits_sum);   // Sum reg

    val1_setter(v_reg_1);
    val2_setter(v_reg_2);
    add(v_reg_1, v_reg_2, c_reg);
    mz(v_reg_1, v_reg_2, c_reg);
  }
};

// Apply not-gates matching binary pattern of val
struct int_setter {
  const long val;
  void operator()(cudaq::qvector<> &qs) __qpu__ {
    for (int i = 1; i <= qs.size(); ++i) {
      auto target_bit_set = (1 << (qs.size() - i)) & val;
      if (target_bit_set) x(qs[i - 1]);
    }
  }
};

int main(int argc, char *argv[]) {
  // Values to add, optionally passed in cmdline
  auto val1 = 1 < argc ? strtol(argv[1], nullptr, 2) : 0b1000;
  auto val2 = 1 < argc ? strtol(argv[2], nullptr, 2) : 0b0100;

  // Calculate necessary bit sizes
  int nbits_val = ceil(log2(max(std::vector<long>({val1, val2})) + 1));
  int nbits_sum = nbits_val+1;

  // Helpful output
  // printf("Input:\n");
  // printf("Nbits_val: %d\n", nbits_val);
  // printf("Nbits_sum: %d\n", nbits_sum);
  printf("Adding values: %ld + %ld (%s + %s)\n", 
          val1, val2, 
          bin_str(val1, nbits_val).c_str(), 
          bin_str(val2, nbits_val).c_str());

  // Generate Circuits and run
  int_setter val1_setter{.val = val1};
  int_setter val2_setter{.val = val2};
  auto counts = cudaq::sample(run_adder{}, nbits_val, nbits_sum, val1_setter, val2_setter);
  std::string result = counts.most_probable();

  // Printing Results
  // printf("Output:\n");
  // auto result_c = result.c_str();
  // std::string val1_out = result.substr(0, nbits_val);
  std::string val2_out = result.substr(nbits_val, nbits_val);
  std::string sum_out = result.substr(2*nbits_val, 1) + val2_out;
  // printf("Found string %s\n", result_c);
  // printf("Val1 Out: %d (%s)\n", bin_to_int(val1_out), val1_out.c_str());
  // printf("Val2 Out: %d (%s)\n", bin_to_int(val2_out), val2_out.c_str());
  printf("Sum: %d (%s)\n", bin_to_int(sum_out), sum_out.c_str());
}

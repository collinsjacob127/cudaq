/**********************************
 * Description: Using Grover's search to find integer 
 *              components that sum to a given value.
 * Author: Jacob Collins
 * Usage:
 * >$ make inverse_add
 * >$ ./inverse_add.o [sum_dec_or_bin] [n_res_to_show]
 * 
 * Examples:
 * >$ ./inverse_add.o 
 *    Finding sum components of: 15
 *    ...
 * 
 * >$ ./inverse_add.o 20
 *   Finding sum components of: 20 (10100)
 *   Q-Alg finished in 7s 139ms 696µs 940ns.
 *   6 + 14 = 20 (5.45%)
 *   11 + 9 = 20 (5.35%)
 *   ...
 *   10 + 10 = 20 (4.20%)
 *   8 + 12 = 20 (4.15%)
 *   2000 / 2000 Correct. (100.00%)
 * 
 * >$ ./inverse_add.o 31 3
 *   Finding sum components of: 31 (11111)
 *   Q-Alg finished in 5s 963ms 167µs 620ns.
 *   30 + 1 = 31 (4.20%)
 *   24 + 7 = 31 (4.15%)
 *   20 + 11 = 31 (4.10%)
 *   More results hidden...
 *   1998 / 2000 Correct. (99.90%)
 **********************************/
#include <cudaq.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

/**************************************************
******************* HELPER FUNCS ******************
***************************************************/

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

  long long remaining_nanoseconds =
      nanoseconds % 1'000'000;  // Remaining nanoseconds
  long long microseconds = remaining_nanoseconds / 1'000;  // From ns to µs
  remaining_nanoseconds = remaining_nanoseconds % 1'000;   // Remaining ns

  // Create the formatted string
  return to_string(seconds) + "s " + to_string(milliseconds) + "ms " +
         to_string(microseconds) + "µs " + to_string(remaining_nanoseconds) +
         "ns";
}

// Check if a string contains only binary characters (0 || 1)
bool is_binary(const std::string& str) {
  for (char c : str) {
    if (c != '0' && c != '1') {
      return false;
    }
  }
  return true;
}

// Check if a string contains only numeric characters
bool is_numeric(const std::string& str) {
  for (char c : str) {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

// Convert an unordered_map to a sorted vector of tuples.
// ( The unordered map is the result of cudaq::sample() )
std::vector<std::tuple<std::string, size_t>> sort_map_by_value_descending(const std::unordered_map<std::string, size_t>& myMap) {
    // Create a vector of tuples from the unordered_map
    std::vector<std::tuple<std::string, size_t>> vec;
    for (const auto& pair : myMap) {
        vec.emplace_back(pair.first, pair.second);
    }

    // Sort the vector in descending order based on the size_t value
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a) > std::get<1>(b); // Compare the size_t values
    });

    return vec;
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

/**************************************************
******************** QUANTUM OPS ******************
***************************************************/

// Apply NOT-gates in accordance with bit-pattern of given integer.
__qpu__ void set_int(const long val, cudaq::qview<> qs) {
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

// Inversion about the mean
struct reflect_uniform {
  void operator()(cudaq::qview<> ctrl, cudaq::qview<> tgt) __qpu__ {
    h(ctrl);
    x(ctrl);
    x(tgt);
    z<cudaq::ctrl>(ctrl, tgt[0]);
    x(tgt);
    x(ctrl);
    h(ctrl);
  }
};

/** ADDER
 * @brief Kernel to perform addition between two qubit registers
 * @param v_reg1 qview - Register of first value
 * @param v_reg2 qview - Register of second value
 * @param c_reg qview - Register for carrying the one.
 * @return void - Our sum is {c_reg[0], v_reg2}
 */
struct adder {
  const int nbits_v;

  void operator()(cudaq::qview<> v_reg1, cudaq::qview<> v_reg2,
                  cudaq::qview<> c_reg) __qpu__ {
    // Carry ones through to c_reg
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
    // Send full output to carry reg
    for (int i = 0; i < nbits_v; ++i) {
      x<cudaq::ctrl>(v_reg2[i], c_reg[i+1]);
    }
  }
};

/**
 * @brief Grover's oracle to search for target_state
 * @param ctrl - Register to search.
 * @param tgt - Qubit on which to apply Toffoli and z-gates.
 */
struct oracle {
  const long target_state;

  void operator()(cudaq::qview<> ctrl, cudaq::qview<> tgt) __qpu__ {
    // Define good search state (secret)
    for (int i = 1; i <= ctrl.size(); ++i) {
      auto target_bit_set = (1 << (ctrl.size() - i)) & target_state;
      if (!target_bit_set)
        x(ctrl[i - 1]);
    }
    // Mark if found
    x<cudaq::ctrl>(ctrl, tgt[0]);
    z(tgt[0]);
    x<cudaq::ctrl>(ctrl, tgt[0]);
    // Undefine good search state
    for (int i = 1; i <= ctrl.size(); ++i) {
      auto target_bit_set = (1 << (ctrl.size() - i)) & target_state;
      if (!target_bit_set)
        x(ctrl[i - 1]);
    }
  }
};

/********************************************
*************** DRIVER & MAIN ***************
*********************************************/

// Driver for adder inversion
struct run_alg {
  __qpu__ auto operator()(const long sum) {
    // 1. Compute Necessary Bits
    // Necessary # bits computed based on input values. Min 1.
    int nbits_val = ceil(log2(max(std::vector<long>({sum, 1})) + 1));
    // Sum register needs 1 extra bit (111 + 111 = 1110)
    int nbits_sum = nbits_val + 1;

    // 2. Initialize Registers
    // cudaq::qvector v_reg1(nbits_val);  // Value 1 reg
    // cudaq::qvector v_reg2(nbits_val);  // Value 2 reg
    cudaq::qvector v_reg(2*nbits_val);  // Values reg. Both 'input' values are stored here.
    cudaq::qvector c_reg(nbits_sum);   // Sum reg
    cudaq::qvector tgt(1);
    adder add_op{.nbits_v = static_cast<int>(nbits_val)};
    reflect_uniform diffuse_op{};
    oracle oracle_op{.target_state = sum};

    // Put our values in superposition
    h(v_reg);
    // set_int(val1, v_reg.front(nbits_val));
    // set_int(val2, v_reg.back(nbits_val));

    // ( pi / 4 ) * sqrt( N / k )
    // N: Size of search space (2^n choose 2)
    // k: Number of valid matching entries (S = {(0, sum), (1, sum-1), ..., (sum, 0)}; k=|S|)
    int n_iter = (0.785398) * sqrt(pow(2, nbits_val)*(pow(2, nbits_val))/sum);
    for (int i = 0; i < n_iter; i++) {
      // 3. Add our value registers
      add_op(v_reg.front(nbits_val), v_reg.back(nbits_val), c_reg);

      // 4. Use oracle to mark our search val
      oracle_op(c_reg, tgt);

      // 5. Undo our addition
      cudaq::adjoint(add_op, v_reg.front(nbits_val), v_reg.back(nbits_val), c_reg);

      // 6. Inversion about the mean to find the right inputs
      diffuse_op(v_reg, tgt); 
    }

    // 7. Measure
    // Sum is {c_reg[0], v_reg2}
    mz(v_reg, c_reg);
  }
};

int main(int argc, char *argv[]) {
  // PARSE INPUT VALUES
  // Default search value
  long search_sum = 0b1111;
  if (argc >= 2) {
    if (is_binary((std::string) argv[1])) {
      search_sum = strtol(argv[1], nullptr, 2);
    } else if (is_numeric((std::string) argv[1])) {
      search_sum = strtol(argv[1], nullptr, 10);
    } else {
      printf("Search value must be given as binary or decimal\n");
    }
  } else {
    printf("A number to find sum components of may be passed as an argument in decimal or binary\n");
  }
  // Necessary # bits computed based on input. Min 1.
  int nbits = ceil(log2(max(std::vector<long>({search_sum, 1})) + 1));
  printf("Finding sum components of: %ld (%s)\n", 
          search_sum, bin_str(search_sum, nbits).c_str());
  printf("Using %d simulated qubits.\n", nbits*3+1);
  int n_grov_iter = (0.785398) * sqrt(pow(2, nbits-1)*(pow(2, nbits-1))/search_sum);
  printf("Grover's requires %d iterations in this case.\n", n_grov_iter);

  // GENERATE AND RUN CIRCUIT
  auto start = std::chrono::high_resolution_clock::now(); // Timer start

  int n_shots = 2000; // Get a lot of samples
  auto counts = cudaq::sample(n_shots, run_alg{}, search_sum);

  auto end = std::chrono::high_resolution_clock::now(); // Timer end
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Q-Alg finished in %s.\n", format_time(duration).c_str());

  // REVIEW RESULTS
  // Counts are an unordered_map
  // Converting to sorted vector of tuples
  std::vector<std::tuple<std::string, size_t>> results = sort_map_by_value_descending(counts.to_map());
  size_t total_correct = 0;
  long n_printed = results.size();
  int i = 0;
  if (argc >= 3) {
    n_printed = strtol(argv[2], nullptr, 10);
  }
  for (const auto& item : results) {
    // Binary result string
    std::string result = std::get<0>(item);
    // Count of this outcome being measured
    size_t count = std::get<1>(item);
    // Parse
    std::string val1_out = result.substr(0, nbits);
    std::string val2_out = result.substr(nbits, nbits);
    int val1 = bin_to_int(val1_out);
    int val2 = bin_to_int(val2_out);
    // % of whole
    if (val1 + val2 == search_sum) {
      total_correct += count;
    }
    if (i < n_printed) {
      printf("%d + %d = %d (%.2f%%)\n", bin_to_int(val1_out), bin_to_int(val2_out), 
              bin_to_int(val1_out) + bin_to_int(val2_out), (float) 100 * count / n_shots);
      // printf("Val1: %d (%s)\n", bin_to_int(val1_out), val1_out.c_str());
      // printf("Val2: %d (%s)\n", bin_to_int(val2_out), val2_out.c_str());
    }
    i++;
  }
  if (n_printed < results.size()) {
    printf("More results hidden...\n");
  }
  // The percentage of results that were correct.
  printf("%lu / %d Correct. (%.2f%%)\n", total_correct, n_shots, (float) 100 * total_correct / n_shots);
}

// Author: Jacob Collins
// Compile and run with:
// ```
// nvq++ grover.cpp -o grover.x && ./grover.x
// ```

// Base includes
#include <cmath>
#include <cudaq.h>
#include <numbers>
// Custom includes
#include <vector>
#include <numeric>
#include <sstream>
#include <string>
#include <iostream>
#include <bitset>

__qpu__ void reflect_about_uniform(cudaq::qvector<> &qs) {
  auto ctrlQubits = qs.front(qs.size() - 1);
  auto &lastQubit = qs.back();

  // Compute (U) Action (V) produces
  // U V U::Adjoint
  cudaq::compute_action(
      [&]() {
        h(qs);
        x(qs);
      },
      [&]() { z<cudaq::ctrl>(ctrlQubits, lastQubit); });
}

struct run_grover {
  template <typename CallableKernel>
  __qpu__ auto operator()(const int n_qubits, CallableKernel &&oracle) {
    int n_iterations = round(0.25 * std::numbers::pi * sqrt(2 ^ n_qubits));

    cudaq::qvector qs(n_qubits);
    h(qs);
    for (int i = 0; i < n_iterations; i++) {
      oracle(qs);
      reflect_about_uniform(qs);
    }
    mz(qs);
  }
};

struct oracle {
  const long target_state;
  const std::vector<long> arr;

  void operator()(cudaq::qvector<> &qs) __qpu__ {
    cudaq::compute_action(
        // Define good search state (secret)
        [&]() {
          for (int i = 1; i <= qs.size(); ++i) {
            auto target_bit_set = (1 << (qs.size() - i)) & target_state;
            if (!target_bit_set)
              x(qs[i - 1]);
          }
        },
        // Controlled z, sends search result to tgt bit
        [&]() {
          auto ctrlQubits = qs.front(qs.size() - 1);
          z<cudaq::ctrl>(ctrlQubits, qs.back());
        });
  }
};

long max(std::vector<long> arr) {
  long max = arr[0];
  for (auto &v : arr) {
    if (v > max) { max = v; }
  }
  return max;
}

template <typename T>
std::string arrayToString(std::vector<T> arr, bool binary, int nbits) {
  std::stringstream ss;
  if (binary) {
    for (int i = 0; i < arr.size(); i++) {
      ss << std::bitset<sizeof(long) * 8>(arr[i]).to_string().substr(sizeof(T)*8-nbits, nbits);
      if (i < arr.size()-1) {
        ss << ", ";;
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

//TODO: Quantum Fourier Transform
// Test that what I wrote here works
__qpu__ void quantum_fourier_transform(cudaq::qvector<> &qs, bool inverse) {
  int N = qs.size();
  cudaq::compute_action(
    // Swap qubits if inverse true
    [&]() {
      if (inverse) {
        for (int i = 0; i < N / 2; ++i) {
          swap(qs[i], qs[N - i - 1]);
        }
      }
    },
    // Apply QFT
    [&]() {
      for (int i = 0; i < N - 1; ++i) {
        h(qs[i]);
        int j = i + 1;
        for (int y = i; y >= 0; --y) {
          double denom = (1UL << (j - y));
          const double theta = -M_PI / denom;
          r1<cudaq::ctrl>(theta, qs[j], qs[y]);
        }
      }
    }
  );
  h(qs[N - 1]); // Python qft didn't have this, hmm
}

//TODO: Test Order
// We have test order in py

//TODO: Modular Mult
// We have modular mult example with 5 and 21 in py

//TODO: Modular Exp
// We have modular exp example with 5 and 21 in py

//TODO: Phase Kernel

//TODO: Order from phase

//TODO: Find Order


void shors(int n, int initial) {
    // int divisor
    if (n % 2 == 0) {
        
    }
    return;
}

int main(int argc, char *argv[]) {
  // Set up the list of values to search through
  std::vector<long> search_vals = {7, 4, 2, 9, 10};
  std::vector<long> index_vals(search_vals.size());
  std::iota(index_vals.begin(), index_vals.end(), 0);

  // Set up value to search for, secret defaults to 3
  auto secret = 1 < argc ? strtol(argv[1], nullptr, 2) : 0b1010;
  int nbits_val = ceil(log2(max(std::vector<long>({max(search_vals), secret})) + 1));
  int nbits_index = ceil(log2(search_vals.size()));
  int nbits = ceil(log2(secret+1));

  // Helpful output
  printf("Search vals: %s\n", arrayToString(search_vals, true, nbits_val).c_str());
  printf("Index vals: %s\n", arrayToString(index_vals, true, nbits_index).c_str());
  printf("Secret: %ld\n", secret);
  printf("Nbits: %d\n", nbits);
  
  // Generate Circuits and run
  oracle compute_oracle{.target_state = secret, .arr = search_vals};
  auto counts = cudaq::sample(run_grover{}, nbits, compute_oracle);
  printf("Found string %s\n", counts.most_probable().c_str());
}

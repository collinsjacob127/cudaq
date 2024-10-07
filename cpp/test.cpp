#include <cudaq.h>
#include <cudaq/algorithm.h>

#include <iostream>

using namespace cudaq::spin;

__qpu__ void kernel(int n_qubits) {
  cudaq::qvector qs(n_qubits);
  h(qs);
}

int main(int argc, char *argv[]) {
  auto qubit_count = 1 < argc ? atoi(argv[1]) : 25;
  auto shots_count = 1000;
  auto start = std::chrono::high_resolution_clock::now();

  // Timing just the sample execution.
  std::cout << "Starting trial now\n";
  auto result = cudaq::sample(shots_count, kernel, qubit_count);

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<double>(stop - start);
  std::cout << "It took " << duration.count() << " seconds.\n";
}

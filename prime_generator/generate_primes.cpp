#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <thread>
#include <mutex>
#include <iomanip>
#include <atomic>
#include "bignum.h"

std::mutex file_mutex;
std::atomic<int> progress;

void print_progress_bar(float percentage) {
    int bar_width = 50;
    std::cout << "[";
    int pos = bar_width * percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setw(6) << std::setprecision(2) << std::fixed << (percentage * 100) << "% completed\r";
    std::cout.flush();
}

void segmented_sieve(BigNum low, BigNum high, const std::vector<BigNum>& small_primes, int n_bits, int segment_size) {
    // Calculate total segments
    BigNum total_numbers = high - low + BigNum(1); // Total numbers in the range
    BigNum total_segments = total_numbers / BigNum(segment_size);
    if (total_numbers % BigNum(segment_size) != BigNum(0)) {
        total_segments = total_segments + BigNum(1); // Add one more segment if there's a remainder
    }

    // Existing code for the segmented sieve
    BigNum size = high - low + BigNum(1);
    std::vector<uint8_t> is_prime((size.data[0] + 7) / 8, 0xFF);

    for (const BigNum& prime : small_primes) {
        BigNum start = std::max(prime * prime, (low + prime - BigNum(1)) / prime * prime);
        for (BigNum j = start; j <= high; j = j + prime) {
            if (j >= low) {
                // Use a mutex here to safely modify shared memory
                std::lock_guard<std::mutex> lock(file_mutex);
                is_prime[(j - low).data[0] / 8] &= ~(1 << ((j - low).data[0] % 8));
            }
        }
    }

    std::stringstream result_buffer;
    for (BigNum i = 0; i < size; i = i + BigNum(1)) {
        if (is_prime[i.data[0] / 8] & (1 << (i.data[0] % 8))) {
            result_buffer << (low + i).toString() << "\n";
        }
    }

    std::lock_guard<std::mutex> lock(file_mutex);
    std::ofstream file;
    std::string filename = "primes/" + std::to_string(n_bits) + "bit_primes_" + low.toString() + "_" + high.toString();
    file.open(filename, std::ios::app);
    file << result_buffer.str();
    file.close();
    // Update progress
    progress.fetch_add(1);
    float percentage = static_cast<float>(progress.load()) / static_cast<float>(total_segments.toInt());
    print_progress_bar(percentage);
}

void generate_primes(BigNum limit, int num_threads, int n_bits) {
    BigNum sqrt_limit = BigNum((uint64_t)sqrt(limit.data[0]));

    std::vector<BigNum> small_primes;
    small_primes.push_back(BigNum(2));
    for (BigNum i = 3; i <= sqrt_limit; i = i + BigNum(2)) {
        bool is_prime = true;
        for (const BigNum& prime : small_primes) {
            if (prime * prime > i) break;
            if (i % prime == BigNum(0)) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) small_primes.push_back(i);
    }

    BigNum segment_size = limit / BigNum(num_threads);
    std::vector<std::thread> threads;

    for (BigNum segment = BigNum(0); segment < limit; segment = segment + segment_size) {
        BigNum low = segment;
        BigNum high = std::min(segment + segment_size - BigNum(1), limit);

        // threads.push_back(std::thread(segmented_sieve, low, high, std::ref(small_primes), n_bits, segment_size));
        // Assuming 'low', 'high', and 'small_primes' are defined appropriately
        threads.push_back(std::thread(segmented_sieve, std::ref(low), std::ref(high), std::ref(small_primes), n_bits, segment_size));

    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Prime number generation complete.\n";
}

int main() {
    progress.store(0);  // Fix for atomic initialization

    int num_bits = 16;
    BigNum limit = BigNum(1) << num_bits;  // Fix for left shift operator
    int num_threads = 4;

    generate_primes(limit, num_threads, num_bits);

    return 0;
}

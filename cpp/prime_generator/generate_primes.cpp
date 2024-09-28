#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <fstream>   // <-- Add this for file handling
#include "bignum.h"

std::mutex file_mutex;
std::atomic<int> progress;

void print_progress_bar(float percentage) {
    int bar_width = 50;
    std::cout << "\r[";
    int pos = bar_width * percentage;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::setw(6) << std::setprecision(2) << std::fixed << (percentage * 100) << "% completed";
    std::cout.flush();  // Ensure the progress bar is printed immediately
}

void segmented_sieve(BigNum low, BigNum high, const std::vector<BigNum>& small_primes, int n_bits, int segment_size, int total_segments) {
    BigNum size = high - low + BigNum(1);
    std::vector<uint8_t> is_prime((size.toInt() + 7) / 8, 0xFF);

    // Mark non-primes using small primes
    for (const BigNum& prime : small_primes) {
        BigNum start = std::max(prime * prime, (low + prime - BigNum(1)) / prime * prime);
        for (BigNum j = start; j <= high; j = j + prime) {
            if (j >= low) {
                is_prime[(j - low).toInt() / 8] &= ~(1 << ((j - low).toInt() % 8));
            }
        }
    }

    // Output primes to a buffer
    std::stringstream result_buffer;
    for (BigNum i = 0; i < size; i = i + BigNum(1)) {
        if (is_prime[i.toInt() / 8] & (1 << (i.toInt() % 8))) {
            result_buffer << (low + i).toString() << "\n";
        }
    }

    // Save results to file
    {
        std::lock_guard<std::mutex> lock(file_mutex);
        std::ofstream file("primes.txt", std::ios::app);
        file << result_buffer.str();
    }

    // Update progress
    int current_progress = progress.fetch_add(1) + 1;  // Fetch and increment
    float percentage = static_cast<float>(current_progress) / static_cast<float>(total_segments);
    print_progress_bar(percentage);
}

void generate_primes(BigNum limit, int num_threads, int n_bits) {
    BigNum sqrt_limit = BigNum(static_cast<uint64_t>(sqrt(limit.toInt())));

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
    int total_segments = num_threads;  // Number of threads = number of segments

    std::vector<std::thread> threads;

    for (BigNum segment = BigNum(0); segment < limit; segment = segment + segment_size) {
        BigNum low = segment;
        BigNum high = std::min(segment + segment_size - BigNum(1), limit);

        threads.push_back(std::thread(segmented_sieve, low, high, std::cref(small_primes), n_bits, segment_size.toInt(), total_segments));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nPrime number generation complete.\n";
}

void example_bignum_use() {
    // Create two 128-bit numbers by concatenating two 64-bit values for each BigNum
    BigNum num1_high(0xFFFFFFFFFFFFFFFF);  // High 64 bits of the first 128-bit number
    BigNum num1_low(0xAAAAAAAAAAAAAAAA);   // Low 64 bits of the first 128-bit number

    BigNum num2_high(0xBBBBBBBBBBBBBBBB);  // High 64 bits of the second 128-bit number
    BigNum num2_low(0xCCCCCCCCCCCCCCCC);   // Low 64 bits of the second 128-bit number

    // Combine high and low parts to form full 128-bit numbers
    BigNum num1 = (num1_high << 64) + num1_low;  // Concatenate high and low parts for num1
    BigNum num2 = (num2_high << 64) + num2_low;  // Concatenate high and low parts for num2

    // Add the two 128-bit numbers
    BigNum sum = num1 + num2;

    // Print the result in hexadecimal (iterate over data in reverse)
    std::cout << "Sum of two 128-bit numbers in hexadecimal: 0x";
    
    bool first_chunk = true;  // Track whether it's the first chunk to avoid leading zeros
    for (int i = sum.data.size() - 1; i >= 0; --i) {
        if (first_chunk) {
            // For the first chunk, don't use leading zeros
            std::cout << std::hex << sum.data[i];
            first_chunk = false;
        } else {
            // For subsequent chunks, use leading zeros
            std::cout << std::hex << std::setw(16) << std::setfill('0') << sum.data[i];
        }
    }
    std::cout << std::endl;
}

int main() {
    example_bignum_use();
    // progress.store(0);

    // int num_bits = 24;
    // BigNum limit = BigNum(1) << num_bits;
    // int num_threads = 4;

    // generate_primes(limit, num_threads, num_bits);

    return 0;
}

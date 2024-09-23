#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <algorithm>
#include <queue>
#include <string>
#include <sys/stat.h> // For creating directories
#include <sys/types.h>

std::mutex progress_mutex;
std::atomic<unsigned int> segments_done(0);

// Function to create a directory if it doesn't exist
void create_directory(const std::string &dir) {
    #ifdef _WIN32
    _mkdir(dir.c_str());
    #else
    mkdir(dir.c_str(), 0777);
    #endif
}

// Function to print the progress bar to the terminal
void print_progress(unsigned int current_segment, unsigned int total_segments) {
    std::lock_guard<std::mutex> guard(progress_mutex);
    float progress = (100.0f * current_segment) / total_segments;

    int bar_width = 50;
    std::cout << "\r[";
    int pos = bar_width * progress / 100.0;
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(2) << progress << "% completed";
    std::cout.flush();
}

// Simple sieve to get small primes
std::vector<unsigned int> simple_sieve(unsigned int limit) {
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (unsigned int i = 2; i * i <= limit; ++i) {
        if (is_prime[i]) {
            for (unsigned int j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }

    std::vector<unsigned int> primes;
    for (unsigned int i = 2; i <= limit; ++i) {
        if (is_prime[i]) {
            primes.push_back(i);
        }
    }

    return primes;
}

// Segmented sieve function
void segmented_sieve(unsigned int low, unsigned int high, const std::vector<unsigned int>& small_primes, unsigned int n_bits, unsigned int total_segments) {
    unsigned int size = high - low + 1;
    std::vector<uint8_t> is_prime((size + 7) / 8, 0xFF);

    for (unsigned int prime : small_primes) {
        unsigned int start = std::max(prime * prime, (low + prime - 1) / prime * prime);
        for (unsigned int j = start; j <= high; j += prime) {
            if (j >= low) {
                is_prime[(j - low) / 8] &= ~(1 << ((j - low) % 8));
            }
        }
    }

    // Save primes to a segment file
    std::stringstream result_buffer;
    for (unsigned int i = 0; i < size; ++i) {
        if (is_prime[i / 8] & (1 << (i % 8))) {
            result_buffer << (low + i) << "\n";
        }
    }

    // Save the result to the correct directory
    create_directory("primes");

    std::string filename = "primes/" + std::to_string(n_bits) + "bit_primes_" + std::to_string(low) + "_" + std::to_string(high) + ".txt";
    std::ofstream file(filename, std::ios::trunc);
    if (file.is_open()) {
        if (!result_buffer.str().empty()) {
            file << result_buffer.str();
        }
        file.close();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }

    segments_done++;
    print_progress(segments_done.load(), total_segments);
}

// Main function for generating primes
void generate_primes_by_bits(unsigned int n_bits, unsigned int num_threads) {
    unsigned int low_threshold = 2;
    unsigned int high_threshold = (1U << n_bits) - 1;
    unsigned int limit = std::sqrt(high_threshold);

    // Get small primes using simple sieve
    std::vector<unsigned int> small_primes = simple_sieve(limit);

    unsigned int segment_size = (n_bits >= 32) ? 500000 : 10000000;
    unsigned int total_segments = (high_threshold - low_threshold + segment_size - 1) / segment_size;
    std::vector<std::thread> threads;

    segments_done = 0;

    for (unsigned int low = low_threshold; low <= high_threshold; low += segment_size) {
        unsigned int high = std::min(low + segment_size - 1, high_threshold);
        threads.emplace_back(segmented_sieve, low, high, std::ref(small_primes), n_bits, total_segments);

        if (threads.size() >= num_threads) {
            for (auto& t : threads) {
                t.join();
            }
            threads.clear();
        }
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nPrime number generation complete.\n";
}

int main() {
    unsigned int n_bits = 24;  // Set this to 32 or more for testing
    unsigned int num_threads = 8;

    std::ios::sync_with_stdio(false);
    generate_primes_by_bits(n_bits, num_threads);

    return 0;
}

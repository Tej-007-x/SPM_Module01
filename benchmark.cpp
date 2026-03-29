#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <random>      

// Only include the intrinsics header if we are building the AVX2 version
#ifdef ENABLE_AVX2
#include <immintrin.h> 
#endif

// --- Configuration ---
const size_t N = 50000000;
const int P = 256;         
const int NUM_RUNS = 20;   

// --- 1. Deterministic Key Generation ---
void generate_keys(std::vector<uint64_t>& keys) {
    std::mt19937_64 rng(12345);
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }
}

// --- 2. Verification Checksum ---
uint64_t calculate_checksum(const std::vector<uint32_t>& part_ids) {
    uint64_t checksum = 0;
    for (size_t i = 0; i < N; ++i) {
        checksum += part_ids[i];
    }
    return checksum;
}

// =========================================================================
// 3. THE MAPPING KERNELS
// =========================================================================

// PLAIN C++ LOGIC (Always compiled)
void mapping_plain(const uint64_t* keys, uint32_t* part_ids, size_t n, int p) {
    uint64_t mask_val = p - 1; 
    for (size_t i = 0; i < n; ++i) {
        part_ids[i] = keys[i] & mask_val; 
    }
}

// AVX2 INTRINSICS LOGIC (Only compiled when ENABLE_AVX2 is defined)
#ifdef ENABLE_AVX2
void mapping_avx2(const uint64_t* keys, uint32_t* part_ids, size_t n, int p) {
    uint64_t mask_val = p - 1;
    __m256i v_mask = _mm256_set1_epi64x(mask_val);
    
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        __m256i v_keys = _mm256_loadu_si256((__m256i*)&keys[i]);
        __m256i v_res = _mm256_and_si256(v_keys, v_mask);
        
        part_ids[i]   = (uint32_t)_mm256_extract_epi64(v_res, 0);
        part_ids[i+1] = (uint32_t)_mm256_extract_epi64(v_res, 1);
        part_ids[i+2] = (uint32_t)_mm256_extract_epi64(v_res, 2);
        part_ids[i+3] = (uint32_t)_mm256_extract_epi64(v_res, 3);
    }

    for (; i < n; ++i) {
        part_ids[i] = keys[i] & mask_val;
    }
}
#endif
// =========================================================================

// --- 4. The Benchmarking Framework ---
void run_benchmark(const std::string& name, 
                   void (*kernel)(const uint64_t*, uint32_t*, size_t, int), 
                   const std::vector<uint64_t>& keys, 
                   std::vector<uint32_t>& part_ids) {
                   
    std::cout << "\n=== Running Benchmark: " << name << " ===\n";
    std::vector<double> execution_times;
    execution_times.reserve(NUM_RUNS);

    for (int i = 0; i < NUM_RUNS; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        kernel(keys.data(), part_ids.data(), N, P);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> duration = end - start;
        execution_times.push_back(duration.count());
    }

    std::sort(execution_times.begin(), execution_times.end());
    double median_time = (NUM_RUNS % 2 == 0) ? 
        (execution_times[NUM_RUNS / 2 - 1] + execution_times[NUM_RUNS / 2]) / 2.0 : 
        execution_times[NUM_RUNS / 2];

    double sum = std::accumulate(execution_times.begin(), execution_times.end(), 0.0);
    double mean_time = sum / NUM_RUNS;
    double sq_sum = 0.0;
    for (double t : execution_times) sq_sum += (t - mean_time) * (t - mean_time);
    double std_dev = std::sqrt(sq_sum / NUM_RUNS);

    double throughput = N / median_time;

    std::cout << "Median Execution Time: " << median_time << " s\n";
    std::cout << "Standard Deviation:    " << std_dev << " s\n";
    std::cout << "Throughput:            " << std::scientific << std::setprecision(2) << throughput << " elements/s\n";
    std::cout << "Verification Checksum: " << calculate_checksum(part_ids) << "\n";
}

// --- 5. Main Execution ---
int main() {
    std::cout << "Allocating memory for " << N << " elements...\n";
    std::vector<uint64_t> keys(N);
    std::vector<uint32_t> part_ids(N, 0);

    std::cout << "Generating input keys deterministically...\n";
    generate_keys(keys);

    // Run Plain Version
    run_benchmark("Plain C++", mapping_plain, keys, part_ids);

    // Only run AVX2 if the macro is defined
#ifdef ENABLE_AVX2
    std::fill(part_ids.begin(), part_ids.end(), 0);
    run_benchmark("AVX2 Intrinsics", mapping_avx2, keys, part_ids);
#endif

    return 0;
}

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <immintrin.h> // Required header for AVX2 intrinsics

// The AVX2 manual vectorization kernel
void map_partitions_avx2(const uint64_t* __restrict__ keys, 
                         uint32_t* __restrict__ part_id, 
                         size_t N, 
                         uint32_t P) {
    
    uint64_t mask_val = P - 1;
    
    // Create a 256-bit vector with the mask copied 4 times
    __m256i v_mask = _mm256_set1_epi64x(mask_val);

    size_t i = 0;
    
    // Process 4 elements per iteration
    for (; i + 3 < N; i += 4) {
        // Load 4 consecutive 64-bit keys from memory into a 256-bit register
        // Using loadu for safety against unaligned memory
        __m256i v_keys = _mm256_loadu_si256((const __m256i*)&keys[i]);
        
        // Perform the bitwise AND operation on all 4 keys simultaneously
        __m256i v_res = _mm256_and_si256(v_keys, v_mask);

        // Extract the four 64-bit results and store them as 32-bit integers in part_id
        part_id[i]   = (uint32_t)_mm256_extract_epi64(v_res, 0);
        part_id[i+1] = (uint32_t)_mm256_extract_epi64(v_res, 1);
        part_id[i+2] = (uint32_t)_mm256_extract_epi64(v_res, 2);
        part_id[i+3] = (uint32_t)_mm256_extract_epi64(v_res, 3);
    }
    
    // "Tail loop" to handle any remaining elements if N isn't perfectly divisible by 4
    for (; i < N; ++i) {
        part_id[i] = keys[i] & mask_val;
    }
}

int main() {
    // 1. Setup variables identically to the baseline
    const size_t N = 50000000; 
    const uint32_t P = 256; 

    std::cout << "[AVX2 Version] Allocating memory for " << N << " elements..." << std::endl;
    std::vector<uint64_t> keys(N);
    std::vector<uint32_t> part_id(N);

    // 2. Deterministic data generation (same seed as baseline)
    std::cout << "Generating input keys deterministically..." << std::endl;
    std::mt19937_64 rng(12345); 
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }

    // 3. Run the AVX2 mapping and measure execution time
    std::cout << "Starting AVX2 partition mapping..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    map_partitions_avx2(keys.data(), part_id.data(), N, P);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    // 4. Calculate and report metrics 
    double seconds = diff.count();
    double throughput = N / seconds; 

    std::cout << "Execution Time: " << seconds << " s\n";
    std::cout << "Throughput: " << std::scientific << std::setprecision(2) 
              << throughput << " elements/s\n";

    // 5. Verification Checksum
    uint64_t checksum = 0;
    for (size_t i = 0; i < N; ++i) {
        checksum += part_id[i];
    }
    std::cout << "Verification Checksum: " << checksum << "\n";

    return 0;
}

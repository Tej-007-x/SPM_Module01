#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdint>
#include <iomanip>

// The mapping kernel: Notice the use of __restrict__                                                                                                                                                               
// This tells the compiler the pointers do not overlap in memory,                                                                                                                                                   
// which is a massive hint that helps GCC auto-vectorize the loop.                                                                                                                                                  
void map_partitions(const uint64_t* __restrict__ keys,
                    uint32_t* __restrict__ part_id,
                    size_t N,
                    uint32_t P) {
    // Assuming P is a power of 2, we can use a bitwise AND mask                                                                                                                                                    
    uint32_t mask = P - 1;

    // This is the "hot loop" the compiler should auto-vectorize                                                                                                                                                    
    for (size_t i = 0; i < N; ++i) {
        // Simple bitwise AND hash to map to [0, P)                                                                                                                                                                 
        part_id[i] = keys[i] & mask;
    }
}

int main() {
    // 1. Setup variables                                                                                                                                                                                           
    // Use a large N (e.g., 50 million) to obtain stable timings                                                                                                                                                    
    const size_t N = 50000000;
    const uint32_t P = 256; // P is typically a power of two                                                                                                                                                        

    std::cout << "Allocating memory for " << N << " elements..." << std::endl;
    std::vector<uint64_t> keys(N);
    std::vector<uint32_t> part_id(N);

    // 2. Deterministic data generation                                                                                                                                                                             
    std::cout << "Generating input keys deterministically..." << std::endl;
    std::mt19937_64 rng(12345); // Fixed seed for reproducibility                                                                                                                                                   
    for (size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }

    // 3. Run the mapping and measure execution time                                                                                                                                                                
    std::cout << "Starting partition mapping..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    map_partitions(keys.data(), part_id.data(), N, P);

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

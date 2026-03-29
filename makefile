# Compiler and Flags                                                                                                                                                                                           
CXX = g++
CXXFLAGS = -O3 -mavx2

# Targets                                                                                                                                                                                                      
all: NonVec Vec AVX2 BenchNonVec BenchVec BenchAVX2

# 1. Baseline: Auto-vectorization disabled (using Mapper.cpp)                                                                                                                                                  
NonVec: Mapper.cpp
        $(CXX) -O3 -fno-tree-vectorize Mapper.cpp -o NonVec -fopt-info-vec 2> nonvec_report.txt

# 2. Auto-vectorized: AVX2 enabled (using Mapper.cpp)                                                                                                                                                          
Vec: Mapper.cpp
        $(CXX) $(CXXFLAGS) -fopt-info-vec Mapper.cpp -o Vec 2> vec_report.txt

# 3. Manual AVX2: (using Mapper_avx2.cpp)                                                                                                                                                                      
AVX2: Mapper_avx2.cpp
        $(CXX) $(CXXFLAGS) -DENABLE_AVX2 Mapper_avx2.cpp -o AVX2 -fopt-info-vec 2> avx_report.txt

# 4. Baseline: Auto-vectorization disabled (using benchmark.cpp)                                                                                                                                               
BenchNonVec: benchmark.cpp
        $(CXX) -O3 -fno-tree-vectorize benchmark.cpp -o BenchNonVec -fopt-info-vec 2> bench_nonvec_report.txt

# 5. Auto-vectorized: AVX2 enabled (using benchmark.cpp)                                                                                                                                                       
BenchVec: benchmark.cpp
        $(CXX) $(CXXFLAGS) -fopt-info-vec benchmark.cpp -o BenchVec 2> bench_vec_report.txt

# 6. Manual AVX2: (using benchmark.cpp)                                                                                                                                                                        
BenchAVX2: benchmark.cpp
        $(CXX) $(CXXFLAGS) -DENABLE_AVX2 benchmark.cpp -o BenchAVX2 -fopt-info-vec 2> bench_avx_report.txt

clean:
        rm -f NonVec Vec AVX2 BenchNonVec BenchVec BenchAVX2 *.txt


Project: SPM Modulo 1 - Vectorization of the Partition Mapping Kernel Author: Teja Karuku
----------------------------------------------------------------------

FILES INCLUDED:
- Mapper.cpp: Plain C++ implementation (used for NonVec and Vec versions).
- Mapper_avx2.cpp: Dedicated manual AVX2 SIMD implementation.
- benchmark.cpp: Unified source code for final statistical benchmarking.
- Makefile: Automated build script for all six binaries and reports.
- README.txt: Setup and execution instructions.
- Report.pdf: 4-page analysis, design choices, and performance results.

1) TRANSFERRING TO THE CLUSTER
From your local terminal, move the project folder to the cluster:
    scp -r ./spm_module01(path of the file) <username>@spmcluster.unipi.it:~/ spm_Module01(Rename the file name, if you want)

2) ACCESSING THE CLUSTER
    ssh <username>@spmcluster.unipi.it
    cd spm_module01

3) COMPILATION AND AUTOMATED CHECK
You can compile all versions and generate optimization reports using:
    make

4) MANUAL COMPILATION (Optional)
If preferred, you may compile versions manually to see optimization reports:

- Baseline: g++ -O3 -fno-tree-vectorize benchmark.cpp -o BenchNonVec 2> report.txt
- Vectorized: g++ -O3 -mavx2 -fopt-info-vec benchmark.cpp -o BenchVec 2> report.txt
- Manual AVX2: g++ -O3 -mavx2 -DENABLE_AVX2 benchmark.cpp -o BenchAVX2

5) EXECUTION ON NODE09
To obtain the stable results used in the report, benchmarks must be 
run on node09. Allocate an interactive session:

    srun --partition=gpu-excl --pty bash
    OR
    srun --partition=gpu-shared --pty bash

Once on node09, run the statistical benchmarks:
    ./BenchNonVec
    ./BenchVec
    ./BenchAVX2

CORRECTNESS AND VERIFICATION:
- Input: 50,000,000 uint64_t keys generated with a deterministic seed.
- Checksum: All versions implement a summation-based verification strategy.
- Expected Result: A consistent checksum of 6375328077 across all versions.
- Statistics: Results report median time and standard deviation over 20 runs.

Two 3d Array Parallel Sum in CPU and GPU  
=====================================================

## CPU Parallel Sum Using fork() and sched_setaffinity()

Instructions
------------
1. Compile the program using ``` gcc -o parallel parallel.c ```
2. Run the program using ``` ./parallel <test_count> <cpu_cores> <array_size>```
3. The program will run the test_count number of times and will print the average time taken for each test.

Example
-------
``` ./parallel 10 4 250 ```
This will run the program 10 times using 4 cpu cores and an array size of 250. The program will print the average time taken for each test.

Problems
----
1. The program has not any reduction operation. It just sums the array elements in parallel. 


## GPU Matrix Parallel Sum using CUDA

Instructions
------------
1. Compile the program using ```nvcc -o parallelGPU parallelGPU.cu```
2. Run the program using ``` ./parallel <test_count> <block_size> <array_size>``` Use block_size as a multiple of 16. 
3. The program will run the test_count number of times and will print the average time taken for each test.

Example
-------
``` ./parallel 10 256 250 ```
This will run the program 10 times using 256 threads per block and an array size of 250. The program will print the average time taken for each test.

Problems
----
1. The program has not any reduction operation. In tests after 10x10x10 matrix, the program gives wrong results in serial sum. In serial sum program uses only one block. It can be improved by using multiple blocks.
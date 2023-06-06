Two 3d Array Parallel Sum in CPU and GPU  
=====================================================

## CPU Parallel Sum Using fork() and sched_setaffinity()

Method
------
It is aimed to allocate the matrices in row-wise order. Each processor cores has rows as a child process by using of sched_affinity and fork(). Each child process has a predefined partner
memory areas will be accessed in parallel, as it will provide a total of one memory area.

Instructions
------------
1. Compile the program using ``` gcc -o parallel parallel.c ```
2. Run the program using ``` ./parallel <test_count> <cpu_cores> <array_size>```
3. The program will run the test_count number of times and will print the average time taken for each test.

Example
-------
``` ./parallel 10 4 250 ```
This will run the program 10 times using 4 cpu cores and an array size of 250. The program will print the average time taken for each test.


Performance
----
# Measure of time
The sum of each matrix value was tested 100 times and the average time was taken according to the number of tests.
| Matrix Size  | 1 Core      | 2 Core      | 3 Core      | 4 Core     |
|--------------|-------------|-------------|-------------|------------|
| 10x10x10     | 0.09751 ms  | 0.2157 ms   | 0.2293 ms   | 0.299 ms   |
| 50x50x50     | 0.7212 ms   | 0.6082 ms   | 0.569 ms    | 0.561 ms   |
| 100x100x100  | 6.3192 ms   | 3.5555 ms   | 2.7162 ms   | 2.318 ms   |
| 200x200x200  | 48.6486 ms  | 26.9858 ms  | 21.565 ms   | 17.027 ms  |
| 500x500x500  | 761.091 ms  | 435.585 ms  | 307.601 ms  | 271.759 ms |

# Speed-up
Speed up chart is given below.

![CPU Speedup Chart](https://imgur.com/scuy7Hs.png)

Problems
----
The program has not any reduction operation. It just sums the array elements in parallel. Therefore max array size is related to system memory size.


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
1. The program has not any reduction operation. 
2. In tests after 10x10x10 matrix, the program gives wrong results in serial sum. In serial sum program uses only one block. It can be improved by using multiple blocks.

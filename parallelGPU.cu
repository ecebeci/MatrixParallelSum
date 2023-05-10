#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define BILLION 1000000000.0
int array_size;
int test_count;
int block_size = 16;

void init_matrix(float *, int);
__global__ void matrix_add(float *, float *, float *);
bool matricesSumChecker(float *, float *, float *, int);

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        printf("Kullanim: %s <test_count> <block_size> <array_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Parse the arguments
    test_count = atoi(argv[1]);
    array_size = atoi(argv[3]);

    // Define matrix size
    int size = array_size * array_size * array_size;

    // Allocate memory for matrices on the device
    float *d_A, *d_B, *d_C1, *d_C2;
    cudaMalloc((void **)&d_A, size * sizeof(float));
    cudaMalloc((void **)&d_B, size * sizeof(float));
    cudaMalloc((void **)&d_C1, size * sizeof(float));
    cudaMalloc((void **)&d_C2, size * sizeof(float));

    // Initialize matrices with random values
    float *h_A = (float *)malloc(size * sizeof(float));
    float *h_B = (float *)malloc(size * sizeof(float));

    init_matrix(h_A, size);
    init_matrix(h_B, size);

    // Copy matrices from host to device
    cudaMemcpy(d_A, h_A, size * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size * sizeof(float), cudaMemcpyHostToDevice);

    // Calculate number of blocks needed for matrix addition
    int num_blocks = (size + block_size - 1) / block_size;

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // Record start event
    cudaEventRecord(start);
    for (int i = 0; i < test_count; i++)
    {
        // Call kernel function to perform matrix addition in parallel
        matrix_add<<<num_blocks, block_size>>>(d_A, d_B, d_C1);
    }
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float ms;
    cudaEventElapsedTime(&ms, start, stop);
    ms /= test_count;
    printf("Paralel Execution time: %f ms\n", ms);

    // Copy result matrix from device to host
    float *h_C1 = (float *)malloc(size * sizeof(float));
    cudaMemcpy(h_C1, d_C1, size * sizeof(float), cudaMemcpyDeviceToHost);

    // Print result matrix
    printf("Check parallel matrices sum: ");
    if (matricesSumChecker(h_A, h_B, h_C1, array_size))
        printf("SUCCESS\n");
    else
        printf("FAILED\n");

    // Serial
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // Record start event
    cudaEventRecord(start);
    for (int i = 0; i < test_count; i++)
    {
        // Call kernel function to perform matrix addition in serial
        matrix_add<<<1, size>>>(d_A, d_B, d_C2);
    }

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&ms, start, stop);
    ms /= test_count;
    printf("Serial Execution time: %f ms\n", ms);

    // Copy result matrix from device to host
    float *h_C2 = (float *)malloc(size * sizeof(float));
    cudaMemcpy(h_C2, d_C2, size * sizeof(float), cudaMemcpyDeviceToHost);

    // Print result matrix
    printf("Check matrices: ");
    if (matricesSumChecker(h_A, h_B, h_C2, array_size))
        printf("SUCCESS\n");
    else
        printf("FAILED\n");

    // Free memory on host and device
    free(h_A);
    free(h_B);
    free(h_C1);
    free(h_C2);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C1);
    cudaFree(d_C2);

    return 0;
}

// Function to initialize matrices with random values
void init_matrix(float *matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        matrix[i] = (float)rand() / RAND_MAX;
    }
}

// Kernel function to perform  matrix addition
__global__ void matrix_add(float *A, float *B, float *C)
{
    // Calculate global index
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    // Add corresponding elements of A and B and store result in C
    C[idx] = A[idx] + B[idx];
}

bool matricesSumChecker(float *M1, float *M2, float *MResult, int array_size)
{
    float *MSum = (float *)malloc(array_size * array_size * array_size * sizeof(float));
    for (int i = 0; i < array_size; i++)
    {
        for (int j = 0; j < array_size; j++)
        {
            for (int k = 0; k < array_size; k++)
            {
                MSum[i * array_size * array_size + j * array_size + k] = M1[i * array_size * array_size + j * array_size + k] + M2[i * array_size * array_size + j * array_size + k];
                if (fabs(MSum[i * array_size * array_size + j * array_size + k] - MResult[i * array_size * array_size + j * array_size + k]) > 1e-5)
                {
                    return false;
                }
            }
        }
    }
    return true;
}
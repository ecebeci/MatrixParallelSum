#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#define BILLION 1000000000.0

int cpu_cores;
int array_size;
int test_count;

int ***a;
int ***b;
int ***sum;

cpu_set_t cpu_mask;

void randArray()
{
    for (int i = 0; i < array_size; i++)
        for (int j = 0; j < array_size; j++)
            for (int k = 0; k < array_size; k++)
            {
                a[i][j][k] = rand() % 100; // Generate number between 0 to 99
                b[i][j][k] = rand() % 100;
            }
}

void parallelSum()
{
    sum = (int ***)mmap(NULL, sizeof(int **) * array_size + sizeof(int *) * array_size * array_size + sizeof(int) * array_size * array_size * array_size,
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    int **row_ptrs = (int **)(sum + array_size);
    int *data = (int *)(row_ptrs + array_size * array_size);

    for (int i = 0; i < array_size; i++)
    {
        sum[i] = row_ptrs + i * array_size;

        for (int j = 0; j < array_size; j++)
        {
            sum[i][j] = data + (i * array_size + j) * array_size;
        }
    }

    int rowPerCore = array_size / cpu_cores;
    int remainingRows = array_size % cpu_cores;

    pid_t pid[array_size];

    for (int core = 0; core < cpu_cores; core++)
    {
        CPU_ZERO(&cpu_mask);
        CPU_SET(core, &cpu_mask);
        if (sched_setaffinity(getpid(), sizeof(cpu_mask), &cpu_mask) == -1)
        {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }
        pid[core] = fork();

        if (pid[core] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[core] == 0)
        {
            // Child process
            int startRow = core * rowPerCore;
            int endRow = (core + 1) * rowPerCore + (core == cpu_cores - 1 ? remainingRows : 0); // (i + 1) * rowPerCore;
            for (int i = startRow; i < endRow; i++)
            {
                for (int j = 0; j < array_size; j++)
                {
                    for (int k = 0; k < array_size; k++)
                    {
                        sum[i][j][k] = a[i][j][k] + b[i][j][k];
                    }
                }
            }

            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < array_size; i++)
    {
        waitpid(pid[i], NULL, 0);
    }

    munmap(sum, sizeof(int **) * array_size + sizeof(int *) * array_size * array_size + sizeof(int) * array_size * array_size * array_size);
}

// TODO: Checker oncesi 2 tanesi mem'e aktarildigi icin boyutlar RAMe göre sınırlı. (WSL de 500den yukarı cıkmadı)
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Kullanim: %s <test_count> <cpu_cores> <array_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Parse the arguments
    test_count = atoi(argv[1]);
    cpu_cores = atoi(argv[2]);
    array_size = atoi(argv[3]);

    // Allocate memory for the arrays
    a = (int ***)malloc(sizeof(int **) * array_size);
    b = (int ***)malloc(sizeof(int **) * array_size);
    sum = (int ***)malloc(sizeof(int **) * array_size);

    for (int i = 0; i < array_size; i++)
    {
        a[i] = (int **)malloc(sizeof(int *) * array_size);
        b[i] = (int **)malloc(sizeof(int *) * array_size);
        sum[i] = (int **)malloc(sizeof(int *) * array_size);

        for (int j = 0; j < array_size; j++)
        {
            a[i][j] = (int *)malloc(sizeof(int) * array_size);
            b[i][j] = (int *)malloc(sizeof(int) * array_size);
            sum[i][j] = (int *)malloc(sizeof(int) * array_size);
        }
    }
    double time_spent;
    struct timespec start, end;
    randArray();

    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i < test_count; i++)
    {
        parallelSum();
    }

    clock_gettime(CLOCK_REALTIME, &end);
    time_spent = (end.tv_sec - start.tv_sec) +
                 (end.tv_nsec - start.tv_nsec) / BILLION;
    time_spent /= test_count;
    printf("Paralel toplama sonucu ortalamasi: %f\n", time_spent);

    return 0;
}
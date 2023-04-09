#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define CPU_CORE 4
#define ARRAY_SIZE 255
cpu_set_t cpu_mask;

int a[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];
int b[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];
int sum2[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];
int ***sum;

void randArray()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
        for (int j = 0; j < ARRAY_SIZE; j++)
            for (int k = 0; k < ARRAY_SIZE; k++)
            {
                a[i][j][k] = rand() % 100; // Generate number between 0 to 99
                b[i][j][k] = rand() % 100;
            }
}

void forkExample()
{
    sum = (int ***)mmap(NULL, sizeof(int **) * ARRAY_SIZE + sizeof(int *) * ARRAY_SIZE * ARRAY_SIZE + sizeof(int) * ARRAY_SIZE * ARRAY_SIZE * ARRAY_SIZE,
                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    int **row_ptrs = (int **)(sum + ARRAY_SIZE);
    int *data = (int *)(row_ptrs + ARRAY_SIZE * ARRAY_SIZE);

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        sum[i] = row_ptrs + i * ARRAY_SIZE;

        for (int j = 0; j < ARRAY_SIZE; j++)
        {
            sum[i][j] = data + (i * ARRAY_SIZE + j) * ARRAY_SIZE;
        }
    }

    int rowPerCore = ARRAY_SIZE / CPU_CORE;
    int remainingRows = ARRAY_SIZE % CPU_CORE;

    pid_t pid[ARRAY_SIZE];

    for (int core = 0; core < CPU_CORE; core++)
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
            int endRow = (core + 1) * rowPerCore + (core == CPU_CORE - 1 ? remainingRows : 0); // (i + 1) * rowPerCore;
            for (int i = startRow; i < endRow; i++)
            {
                for (int j = 0; j < ARRAY_SIZE; j++)
                {
                    for (int k = 0; k < ARRAY_SIZE; k++)
                    {
                        sum[i][j][k] = a[i][j][k] + b[i][j][k];
                    }
                }
            }

            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        waitpid(pid[i], NULL, 0);
    }
}

int notParallelExample()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < ARRAY_SIZE; j++)
        {
            for (int k = 0; k < ARRAY_SIZE; k++)
            {
                sum2[i][j][k] = a[i][j][k] + b[i][j][k];
            }
        }
    }
}

int duplicateChecker()
{
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < ARRAY_SIZE; j++)
        {
            for (int k = 0; k < ARRAY_SIZE; k++)
            {
                if (sum2[i][j][k] != sum[i][j][k])
                    return 1;
            }
        }
    }
    return 0;
}

int main()
{
    double time_spent;
    clock_t begin, end;

    randArray();
    begin = clock();
    forkExample();
    end = clock();

    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%f \n", time_spent);

    begin = clock();
    notParallelExample();
    end = clock();

    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%f \n", time_spent);

    int checker = duplicateChecker(sum, sum2);
    printf("Esitlik Sonuc: %i \n", checker);

    return 0;
}
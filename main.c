#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define ARRAY_SIZE 50
cpu_set_t cpu_mask;
// const int array_dimension = 2;

// shared memory kullanmilmali. Cunku child olusturulunca parent bellegini kopyalar ve kendi bellegine aktatarir

// int a[3][3][3] = {{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}},
//                   {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}},
//                   {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}}};

// int b[3][3][3] = {{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}},
//                   {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}},
//                   {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}}};

int a[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];
int b[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];

int sum2[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];

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

    int ***sum = (int ***)mmap(NULL, sizeof(int *) * ARRAY_SIZE, PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        sum[i] = (int **)mmap(NULL, sizeof(int **) * ARRAY_SIZE, PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        for (int j = 0; j < ARRAY_SIZE; j++)
        {
            sum[i][j] = (int *)mmap(NULL, sizeof(int) * ARRAY_SIZE, PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        }
    }

    pid_t child_pids[ARRAY_SIZE];

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        pid_t pid = fork();
        // child process because return value zero
        if (pid == 0)
        {
            CPU_ZERO(&cpu_mask);
            CPU_SET(i, &cpu_mask);
            if (sched_setaffinity(getpid(), sizeof(cpu_mask), &cpu_mask) == -1)
            {
                perror("sched_setaffinity");
                exit(EXIT_FAILURE);
            }

            printf("CPU => %d, Child process => PPID=%d, PID=%d\n", i, getppid(), getpid());
            for (int j = 0; j < ARRAY_SIZE; j++)
            {
                for (int k = 0; k < ARRAY_SIZE; k++)
                {
                    sum[i][j][k] = a[i][j][k] + b[i][j][k];
                    // printf("Child Sum[%d][%d][%d] %d \n", i, j, k, sum[i][j][k]);
                }
            }
            exit(0);
        }
        else if (pid > 0)
        {
            child_pids[i] = pid;
        }
        else
        {
            printf("Unable to create child process.\n");
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        waitpid(child_pids[i], NULL, 0);
    }

    // for (size_t i = 0; i < ARRAY_SIZE; i++)
    // {
    //     for (size_t j = 0; j < ARRAY_SIZE; j++)
    //     {
    //         for (size_t k = 0; k < ARRAY_SIZE; k++)
    //         {
    //             printf("Sum[%ld][%ld][%ld] = %d\n", i, j, k, sum[i][j][k]);
    //         }
    //     }
    //     printf("\n");
    // }

    // Clean up
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        for (int j = 0; j < ARRAY_SIZE; j++)
        {
            munmap(sum[i][j], sizeof(int) * ARRAY_SIZE); // deletes [][][THIS]
        }
        munmap(sum[i], sizeof(int) * ARRAY_SIZE); // deletes [][THIS]
    }
    munmap(sum, sizeof(int *) * ARRAY_SIZE); // deletes [THIS]
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

// int check()
// {
//     for (int i = 0; i < ARRAY_SIZE; i++)
//     {
//         for (int j = 0; j < ARRAY_SIZE; j++)
//         {
//             for (int k = 0; k < ARRAY_SIZE; k++)
//             {
//               if(sum[i][j][k])
//             }
//         }
//     }
// }

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
    return 0;
}
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

#define CPU_CORE 4

cpu_set_t cpu_mask;

typedef struct Tensor
{
    size_t rows;
    size_t cols;
    size_t tubes;
    int ***data;
} Tensor;

Tensor *createT(size_t rows, size_t cols, size_t tubes)
{
    Tensor *t = malloc(sizeof(Tensor));
    t->rows = rows;
    t->cols = cols;
    t->tubes = tubes;
    t->data = malloc(rows * sizeof(int **));
    for (size_t i = 0; i < rows; i++)
    {
        t->data[i] = malloc(cols * sizeof(int *));
        for (size_t j = 0; j < cols; j++)
        {
            t->data[i][j] = malloc(tubes * sizeof(int));
        }
    }
    return t;
}

Tensor *initT(Tensor *t)
{
    for (size_t i = 0; i < t->rows; i++)
    {
        for (size_t j = 0; j < t->cols; j++)
        {
            for (size_t k = 0; k < t->tubes; k++)
            {
                t->data[i][j][k] = rand();
            }
        }
    }
    return t;
}

void freeT(Tensor *t)
{
    for (size_t i = 0; i < t->rows; i++)
    {
        for (size_t j = 0; j < t->cols; j++)
        {
            free(t->data[i][j]);
        }
        free(t->data[i]);
    }
    free(t->data);
    free(t);
}

Tensor *partialRowSumT(Tensor *t1, Tensor *t2, size_t startRow, size_t endRow)
{
    Tensor *sum = createT(t1->cols, t1->rows, t1->tubes);

    for (size_t i = startRow; i < endRow; i++)
    {
        for (size_t j = 0; j < t1->cols; j++)
        {
            for (size_t k = 0; k < t1->tubes; k++)
            {
                sum->data[i][j][k] = t1->data[i][j][k] + t2->data[i][j][k];
            }
        }
    }
    return sum;
}

Tensor *parallelSumT(Tensor *t1, Tensor *t2)
{
    Tensor *sum = createT(t1->rows, t1->cols, t1->tubes);

    int rowPerCore = t1->rows / CPU_CORE;
    int remainingRows = t1->rows % CPU_CORE;

    pid_t pid[CPU_CORE];
    int status;

    for (int i = 0; i < CPU_CORE; i++)
    {
        CPU_ZERO(&cpu_mask);
        CPU_SET(i, &cpu_mask);
        if (sched_setaffinity(getpid(), sizeof(cpu_mask), &cpu_mask) == -1)
        {
            perror("sched_setaffinity");
            exit(EXIT_FAILURE);
        }
        pid[i] = fork();

        if (pid[i] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid[i] == 0)
        {
            // Child process
            int startRow = i * rowPerCore;
            int endRow = (i + 1) * rowPerCore + (i == CPU_CORE - 1 ? remainingRows : 0); // (i + 1) * rowPerCore;
 
            partialRowSumT(t1, t2, startRow, endRow);

            freeT(t1);
            freeT(t2);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes to complete
    for (int i = 0; i < CPU_CORE; i++)
    {
        waitpid(pid[i], &status, 0);
    }

    return sum;
}

// Paralel olmayan işlemlerde kullanılır
Tensor *sumSingleCoreT(Tensor *t1, Tensor *t2)
{
    Tensor *sum = createT(t1->rows, t1->cols, t1->tubes);

    for (size_t i = 0; i < t1->rows; i++)
    {
        for (size_t j = 0; j < t1->cols; j++)
        {
            for (size_t k = 0; k < t1->tubes; k++)
            {
                sum->data[i][j][k] = t1->data[i][j][k] + t2->data[i][j][k];
            }
        }
    }

    return sum;
}

int main()
{
    double time_spent;
    clock_t begin, end;

    Tensor *t1 = createT(1000, 100, 1000);
    Tensor *t2 = createT(1000, 100, 1000);
    initT(t1);
    initT(t2);
    t1->data[0][0][0] = 3;
    t2->data[0][0][0] = 5;

    begin = clock();
    Tensor *sum = parallelSumT(t1, t2);
    end = clock();

    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Birdan fazla çekirdek ile: %f \n", time_spent);

    begin = clock();
    sumSingleCoreT(t1, t2);
    end = clock();

    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Tek Core ile yapılan işlem: %f \n", time_spent);

    // Tensor *t = partialRowSumT(t1, t2, 0, 2);

    // printf("%li %li %li\n", t->rows, t->cols, t->tubes);
    // for (int i = 0; i < 2; i++)
    // {
    //     for (int j = 0; j < 2; j++)
    //     {
    //         for (int k = 0; k < 2; k++)
    //         {
    //             printf("%i \t", t->data[i][j][k]);
    //         }
    //     }
    //     printf("\n");
    // }
}

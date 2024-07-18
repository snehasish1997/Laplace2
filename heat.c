#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

double get_final_temperatures(int N, int maxIter, double radTemp) {
    int i, j, iter;
    int centerX = N / 2, centerY = N / 2;

    // Allocate 2D arrays for current and previous temperature states
    double **prev_t = (double **)malloc(N * sizeof(double *));
    double **curr_t = (double **)malloc(N * sizeof(double *));
    for (i = 0; i < N; i++) {
        prev_t[i] = (double *)malloc(N * sizeof(double));
        curr_t[i] = (double *)malloc(N * sizeof(double));
    }

    // Initialize the arrays with initial conditions
    #pragma omp parallel for private(j)
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            prev_t[i][j] = 10.0; // initial temperature
            curr_t[i][j] = 10.0; // initial temperature
        }
    }

    // Set radiator temperature
    int radStart = floor((N - 1) * 0.3);
    int radEnd = ceil((N - 1) * 0.7);
    for (i = radStart; i <= radEnd; i++) {
        prev_t[N-1][i] = radTemp;
        curr_t[N-1][i] = radTemp;
    }

    // Iteratively update temperature distribution
    for (iter = 0; iter < maxIter; iter++) {
        #pragma omp parallel for private(j)
        for (i = 1; i < N - 1; i++) {
            for (j = 1; j < N - 1; j++) {
                curr_t[i][j] = 0.25 * (prev_t[i+1][j] + prev_t[i-1][j] + prev_t[i][j+1] + prev_t[i][j-1]);
            }
        }
        // Swap current and previous arrays
        double **temp = prev_t;
        prev_t = curr_t;
        curr_t = temp;
    }

    double result = prev_t[centerX][centerY];

    // Free allocated memory
    for (i = 0; i < N; i++) {
        free(prev_t[i]);
        free(curr_t[i]);
    }
    free(prev_t);
    free(curr_t);

    return result;
}

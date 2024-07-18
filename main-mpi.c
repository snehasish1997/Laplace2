#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "file-reader.h"

extern double get_final_temperatures(int N, int maxIter, double radTemp);

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <N> <max_iter> <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]);
    int maxIter = atoi(argv[2]);
    char *inputFile = argv[3];
    char *outputFile = argv[4];

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int numTemps;
    double *radTemps = NULL;

    if (rank == 0) {
        // Read the number of radiator temperatures
        numTemps = read_num_of_temps(inputFile);
        if (numTemps <= 0) {
            fprintf(stderr, "Error reading number of temperatures from file: %s\n", inputFile);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Read the radiator temperatures
        radTemps = read_temps(inputFile, numTemps);
        if (radTemps == NULL) {
            fprintf(stderr, "Error reading temperatures from file: %s\n", inputFile);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    // Broadcast the number of temperatures to all processes
    MPI_Bcast(&numTemps, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate local number of temperatures
    int localNumTemps = numTemps / size;
    if (rank == size - 1) {
        localNumTemps += numTemps % size; // Add remainder to the last rank
    }

    // Allocate memory for local radiator temperatures and results
    double *localRadTemps = (double *)malloc(localNumTemps * sizeof(double));
    double *localResults = (double *)malloc(localNumTemps * sizeof(double));

    // Scatter the radiator temperatures to all processes
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    int tempSize = numTemps / size;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = tempSize;
        if (i == size - 1) {
            sendcounts[i] += numTemps % size; // Add remainder to the last rank
        }
        displs[i] = i * tempSize;
    }

    MPI_Scatterv(radTemps, sendcounts, displs, MPI_DOUBLE, localRadTemps, localNumTemps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Compute the final temperatures for each local radiator temperature
    for (int i = 0; i < localNumTemps; i++) {
        localResults[i] = get_final_temperatures(N, maxIter, localRadTemps[i]);
    }

    // Gather the results from all processes
    double *results = NULL;
    if (rank == 0) {
        results = (double *)malloc(numTemps * sizeof(double));
    }
    MPI_Gatherv(localResults, localNumTemps, MPI_DOUBLE, results, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Write results to output file
    if (rank == 0) {
        write_to_output_file(outputFile, results, numTemps);
        free(results);
        free(radTemps);
    }

    // Free allocated memory
    free(localRadTemps);
    free(localResults);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return EXIT_SUCCESS;
}

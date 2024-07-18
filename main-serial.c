#include <stdio.h>
#include <stdlib.h>
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

    // Read the number of radiator temperatures
    int numTemps = read_num_of_temps(inputFile);
    if (numTemps <= 0) {
        fprintf(stderr, "Error reading number of temperatures from file: %s\n", inputFile);
        return EXIT_FAILURE;
    }

    // Read the radiator temperatures
    double *radTemps = read_temps(inputFile, numTemps);
    if (radTemps == NULL) {
        fprintf(stderr, "Error reading temperatures from file: %s\n", inputFile);
        return EXIT_FAILURE;
    }

    // Allocate array for results
    double *results = (double *)malloc(numTemps * sizeof(double));

    // Compute the final temperatures for each radiator temperature
    for (int i = 0; i < numTemps; i++) {
        results[i] = get_final_temperatures(N, maxIter, radTemps[i]);
    }

    // Write results to output file
    write_to_output_file(outputFile, results, numTemps);

    // Free allocated memory
    free(radTemps);
    free(results);

    return EXIT_SUCCESS;
}

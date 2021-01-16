/*
 * Author: Bureaca Emil
 */
#include <stdio.h>
#include <stdlib.h>

const int MIN_PLANET_ROOT = 2;
const int MAX_PLANET_ROOT = 20;

int main(int argc, char *argv[]){

    FILE *output_file;
    int GALAXY_WIDTH, GALAXY_HEIGHT, PLANET_SIZE_ROOT, PLANET_SIZE, p;

    if (argc < 5){
        fprintf(stdout, "Usage: %s GALAXY_WIDTH GALAXY_HEIGHT OUTPUT_FILENAME RANDOM_SEED\n", argv[0]);
        exit(1);
    }

    GALAXY_WIDTH = atoi(argv[1]);
    GALAXY_HEIGHT = atoi(argv[2]);
    if(GALAXY_HEIGHT < 1 || GALAXY_WIDTH < 1){
        fprintf(stdout, "Error: GALAXY_HEIGHT and GALAXY_WIDTH must be greater than 0.\n");
        fprintf(stdout, "Usage: %s GALAXY_WIDTH GALAXY_HEIGHT OUTPUT_FILENAME RANDOM_SEED\n", argv[0]);
        exit(1);
    }

    output_file = fopen(argv[3], "w");
    if (output_file == NULL) {
        fprintf(stdout, "Failed to open the file %s\n.", argv[3]);
        exit(1);
    }

    srand(atoi(argv[4]));

    // Galaxy dimensions
    fprintf(output_file, "%d %d\n", GALAXY_HEIGHT, GALAXY_WIDTH);

    for (int i = 0; i < GALAXY_HEIGHT; i++){
        for (int j = 0; j < GALAXY_WIDTH; j++){

            PLANET_SIZE_ROOT = rand() % (MAX_PLANET_ROOT - MIN_PLANET_ROOT + 1) + MIN_PLANET_ROOT;
            PLANET_SIZE = PLANET_SIZE_ROOT * PLANET_SIZE_ROOT;
            fprintf(output_file, "%d\n", PLANET_SIZE);

            for(p = 0; p < PLANET_SIZE; p++){

                // Food resources
                fprintf(output_file, "%f ", ((double)(rand() % 11)) / 10);

                // Building resources
                fprintf(output_file, "%f ", ((double)(rand() % 11)) / 10);

                // Environment coeficient
                fprintf(output_file, "%f ", ((double)(rand() % 11)) / 10);
                fprintf(output_file, "\n");

            }
        }
    }

    fclose(output_file);

    return 0;

}
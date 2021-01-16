#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../headers/helpers/math_operations.h"
#include "../headers/helpers/logger.h"
#include "../headers/helpers/conditon_checkers.h"

typedef struct zone{
    double food;
    double material;
    double environment;
    double population;
    double infrastructure;
} zone;

typedef struct planet{
    int size;
    zone *zones;
    double population_mean;
} planet;

typedef struct galaxy{
    int height;
    int width;
    planet *planets;
} galaxy;

#define ENABLE_ALL_LOG 0

const double INIT_POPULATION = 120000.0;
const double INIT_INFRASTRUCTURE = 500000.0;
const double K_MG = 0.005;
const double K_IF = 0.003;

char *input_filename;
char *output_filename;
int iterations;
int source_planet_x;
int source_planet_y;

galaxy working_galaxy;

void get_arguments(int argc, char *argv[]){

    if (argc < 6){
        printf("Usage: %s INPUT_FILENAME OUTPUT_FILENAME ITERATIONS SOURCE_PLANET_X SOURCE_PLANET_Y\n", \
            argv[0]);
        exit(-1);
    }

    input_filename = argv[1];
    output_filename = argv[2];
    iterations = atoi(argv[3]);
    source_planet_x = atoi(argv[4]);
    source_planet_y = atoi(argv[5]);

}

inline int get_planet_id(int x, int y){
    return y * working_galaxy.width + x;
}

inline int check_existent_planet(int x, int y){
    return (y >= 0 && y < working_galaxy.height && x >= 0 && x < working_galaxy.width);
}

double compute_planet_population_mean(int x, int y){

    planet *working_planet = NULL;
    double planet_mean = 0;
    int zone_id;

    working_planet = &working_galaxy.planets[get_planet_id(x, y)];
    for (zone_id = 0; zone_id < working_planet->size; zone_id++)
        planet_mean += working_planet->zones[zone_id].population;
    planet_mean /= (double)working_planet->size;

    LOGGER(ENABLE_ALL_LOG, "Average for planet (%d, %d) is %f", x, y, planet_mean);

    return planet_mean;

}

int read_input_file(){

    FILE *input_file = NULL;
    planet *working_planet = NULL;
    zone *working_zone = NULL;
    int is_error = 0, source_planet_id, planets_count, planet_id, zone_id, i;

    input_file = fopen(input_filename, "r");
    GOTO_CONDITION_CHECKER(input_file == NULL, is_error, EXIT_READ_INPUT_FILE_1);

    fscanf(input_file, "%d %d", &working_galaxy.height, &working_galaxy.width);
    planets_count = working_galaxy.height * working_galaxy.width;
    working_galaxy.planets = (planet *)malloc(planets_count * sizeof(planet));
    GOTO_CONDITION_CHECKER(working_galaxy.planets == NULL, is_error, EXIT_READ_INPUT_FILE_2);

    source_planet_id = get_planet_id(source_planet_x, source_planet_y);
    for (planet_id = 0; planet_id < planets_count; planet_id++){

        working_planet =  &working_galaxy.planets[planet_id];

        fscanf(input_file, "%d", &working_planet->size);
        working_planet->zones = (zone *)malloc(working_planet->size * sizeof(zone));
        GOTO_CONDITION_CHECKER(working_planet->zones == NULL, is_error, EXIT_READ_INPUT_FILE_3);

        for (zone_id = 0; zone_id < working_planet->size; zone_id++){

            working_zone = &working_planet->zones[zone_id];

            fscanf(input_file, "%lf %lf %lf", &working_zone->food, &working_zone->material, \
                &working_zone->environment);
            working_zone->population = (planet_id == source_planet_id) ? INIT_POPULATION : 0;
            working_zone->infrastructure = (planet_id == source_planet_id) ? INIT_INFRASTRUCTURE : 0;

        }
        working_planet->population_mean = (planet_id == source_planet_id) ? INIT_POPULATION : 0;

    }

    EXIT_READ_INPUT_FILE_3:
        if (is_error){
            for (i = 0; i < planet_id; i++)
                free(working_galaxy.planets[i].zones);
            free(working_galaxy.planets);
        }
    EXIT_READ_INPUT_FILE_2:
        fclose(input_file);
    EXIT_READ_INPUT_FILE_1:
        return is_error;

}

void manipulate_process_border(int horizontal_processes, int vertical_processes, \
    int process_rank, double *border, int is_write){

    planet *working_planet;
    int column_id, row_id, allocated_width, allocated_height, \
        min_width, min_height, max_width, max_height, i = 0, j;

    // Compute allocated planets
    column_id = process_rank % horizontal_processes;
    row_id = process_rank / horizontal_processes;
    allocated_width = ceil((double)working_galaxy.width / (double)horizontal_processes);
    allocated_height = ceil((double)working_galaxy.height / (double)vertical_processes);
    min_width = column_id * allocated_width;
    max_width = min(working_galaxy.width, (column_id + 1) * allocated_width);
    if (allocated_width != max_width - min_width)
        allocated_width = max_width - min_width;
    min_height = row_id * allocated_height;
    max_height = min(working_galaxy.height, (row_id + 1) * allocated_height);
    if (allocated_height != max_height - min_height)
        allocated_height = max_height - min_height;

    LOGGER(ENABLE_ALL_LOG, "Computed dimensions for process %d are: [%d - %d - %d) and [%d - %d - %d)", \
        process_rank, min_width, allocated_width, max_width, \
        min_height, allocated_height, max_height);

    // Check if the allocated space is just a planet
    if (allocated_width == 1 && allocated_height == 1){
        working_planet = &working_galaxy.planets[get_planet_id(min_width, min_height)];
        if (is_write)
            working_planet->population_mean = border[0];
        else
            border[i] = working_planet->population_mean;
        return;
    }

    // Check if the allocated space is just of a column of the galaxy
    if (allocated_width == 1){
        for (j = 0; j < allocated_height; j++){
            working_planet = &working_galaxy.planets[get_planet_id(min_width, j + min_height)];
            if (is_write)
                working_planet->population_mean = border[i++];
            else
                border[i++] = working_planet->population_mean;
        }
        return;
    }

    // Check if the allocated space is just of a row of the galaxy
    if (allocated_height == 1){
        for (j = 0; j < allocated_width; j++){
            working_planet = &working_galaxy.planets[get_planet_id(j + min_width, min_height)];
            if (is_write)
                working_planet->population_mean = border[i++];
            else
                border[i++] = working_planet->population_mean;
        }
        return;
    }

    // Else, manipulate the border of the given process
    LOGGER(ENABLE_ALL_LOG, "Manipulating top border with size %d", allocated_width - 1);
    for (j = 0; j < allocated_width - 1; j++){
        working_planet = &working_galaxy.planets[get_planet_id(j + min_width, min_height)];
        if (is_write)
            working_planet->population_mean = border[i++];
        else
            border[i++] = working_planet->population_mean;
    }
    LOGGER(ENABLE_ALL_LOG, "Manipulating right border with size %d", allocated_height - 1);
    for (j = 0; j < allocated_height - 1; j++){
        working_planet = &working_galaxy.planets[get_planet_id(max_width - 1, j + min_height)];
        if (is_write)
            working_planet->population_mean = border[i];
        else
            border[i] = working_planet->population_mean;
        i++;
    }
    LOGGER(ENABLE_ALL_LOG, "Manipulating bottom border with size %d", allocated_width - 1);
    for (j = 0; j < allocated_width - 1; j++){
        working_planet = &working_galaxy.planets[get_planet_id(max_width - 1 - j, max_height - 1)];
        if (is_write)
            working_planet->population_mean = border[i++];
        else
            border[i++] = working_planet->population_mean;
    }
    LOGGER(ENABLE_ALL_LOG, "Manipulating left border with size %d", allocated_height - 1);
    for (j = 0; j < allocated_height - 1; j++){
        working_planet = &working_galaxy.planets[get_planet_id(min_width, max_height - 1 - j)];
        if (is_write)
            working_planet->population_mean = border[i++];
        else
            border[i++] = working_planet->population_mean;
    }
    LOG_DOUBLE_ARRAY(ENABLE_ALL_LOG, border, i, "Manipulated border with size %d was:", i);

}

int main(int argc, char *argv[]){

    FILE *output_file = NULL;
    planet *working_planet = NULL, *neighbour_planet = NULL;
    zone *working_zone = NULL;
    double *working_border = NULL;
    double population_growth_factor, old_infrastructure;
    int neighbours_ranks[8];
    int processes_count, rank, horizontal_processes, vertical_processes,
        tiles_count, column_id, row_id, allocated_width, allocated_height, border_size, \
        min_width, max_width, min_height, max_height, \
        interation_id, i, j, l, m, \
        zone_id, ret_val;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes_count);

    // Read arguments
    get_arguments(argc, argv);

    // Read input file in a global data structure
    ret_val = read_input_file();
    RET_CONDITION_CHECKER(ret_val != 0, ret_val);

    // Check galaxy size
    if (working_galaxy.height == 1 && working_galaxy.width == 1){
        if (rank != 0)
            goto MAIN_EXIT;
        else
            processes_count = 1;
    }

    // Divide the matrix into parts
    tiles_count = int_sqrt(processes_count);
    tiles_count += (processes_count % tiles_count) != 0;
    horizontal_processes = gcd(processes_count, tiles_count);
    vertical_processes = processes_count / horizontal_processes;
    if (rank == 0)
        LOGGER(ENABLE_ALL_LOG, "There are %d x %d (horizontal x vertical) processes", \
            horizontal_processes, vertical_processes);

    // Compute allocated planets
    column_id = rank % horizontal_processes;
    row_id = rank / horizontal_processes;
    allocated_width = ceil((double)working_galaxy.width / (double)horizontal_processes);
    allocated_height = ceil((double)working_galaxy.height / (double)vertical_processes);
    min_width = column_id * allocated_width;
    max_width = min(working_galaxy.width, (column_id + 1) * allocated_width);
    min_height = row_id * allocated_height;
    max_height = min(working_galaxy.height, (row_id + 1) * allocated_height);
    LOGGER(ENABLE_ALL_LOG, "Allocation the planets with width [%d, %d) and height [%d, %d)", \
        min_width, max_width, min_height, max_height);

    // Allocate borders
    if (allocated_width == 1 || allocated_height == 1)
        border_size = allocated_width * allocated_height;
    else
        border_size = 2 * (allocated_width + allocated_height) - 4;
    LOGGER(ENABLE_ALL_LOG, "Allocation for border with width of %d", border_size);
    working_border = (double *)malloc(border_size * sizeof(double));
    RET_CONDITION_CHECKER(working_border == NULL, 1);

    // Get real allocated dimensions
    allocated_width = max_width - min_width;
    allocated_height = max_height - min_height;

    // Get neighbors neighbors ranks
    if (processes_count > 1){

        // Set the default value
        for (i = 0; i < 8; i++)
            neighbours_ranks[i] = -1;

        // Top neighbor
        if (min_height > 0)
            neighbours_ranks[1] = rank - horizontal_processes;

        // Bottom neighbor
        if (max_height < working_galaxy.height)
            neighbours_ranks[6] = rank + horizontal_processes;

        // Left neighbor
        if (min_width > 0)
            neighbours_ranks[3] = rank - 1;

        // Right neighbor
        if (max_width < working_galaxy.width)
            neighbours_ranks[4] = rank + 1;

        // Top-left corner neighbor
        if (min_height > 0 && min_width > 0)
            neighbours_ranks[0] = rank - horizontal_processes - 1;

        // Top-right corner neighbor
        if (min_height > 0 && max_width < working_galaxy.width)
            neighbours_ranks[2] = rank - horizontal_processes + 1;

        // Bottom-left corner neighbor
        if (max_height < working_galaxy.height && min_width > 0)
            neighbours_ranks[5] = rank + horizontal_processes - 1;

        // Bottom-right corner neighbor
        if (max_height < working_galaxy.height && max_width < working_galaxy.width)
            neighbours_ranks[7] = rank + horizontal_processes + 1;

        MPI_Barrier(MPI_COMM_WORLD);

    }

    // Compute population and infrastructure
    for (interation_id = 0; interation_id < iterations; interation_id++){

        // Activate the IPC if there are more than one process
        if (processes_count > 1){

            // Read own border and send it to all neighbors
            for (i = 0; i < border_size; i++)
                working_border[i] = -1;
            manipulate_process_border(horizontal_processes, vertical_processes, rank, working_border, 0);
            for (i = 0; i < 8; i++)
                if (neighbours_ranks[i] != -1){
                    LOGGER(ENABLE_ALL_LOG, "Sending border to neighbor %d from direction %d", \
                        neighbours_ranks[i], i);
                    MPI_Send(working_border, border_size, MPI_DOUBLE, neighbours_ranks[i], 0, MPI_COMM_WORLD);
                }

            // Write neighors borders
            for (i = 0; i < 8; i++)
                if (neighbours_ranks[i] != -1){
                    MPI_Recv(working_border, border_size, MPI_DOUBLE, neighbours_ranks[i], 0, \
                        MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    LOGGER(ENABLE_ALL_LOG, "Receiving border from neighbor %d from direction %d", \
                        neighbours_ranks[i], i);
                    manipulate_process_border(horizontal_processes, vertical_processes, \
                        neighbours_ranks[i], working_border, 1);
                }

        }

        LOGGER(ENABLE_ALL_LOG, "Beginning calculations");
        for (i = min_height; i < max_height; i++){
            for (j = min_width; j < max_width; j++){

                // Compute population growth factor
                population_growth_factor = 0;
                for (l = -1; l <= 1; l++){
                    for (m = -1; m <= 1; m++){

                        // Skip the current planet
                        if (l == 0 && m == 0)
                            continue;

                        // Skip planets outsite of the galaxy
                        if (!check_existent_planet(j + m, i + l))
                            continue;

                        neighbour_planet = &working_galaxy.planets[get_planet_id(j + m, i + l)];
                        population_growth_factor += neighbour_planet->population_mean;

                    }
                }
                population_growth_factor *= K_MG;

                // Get current planet and set its new parameters
                working_planet = &working_galaxy.planets[get_planet_id(j, i)];
                for (zone_id = 0; zone_id < working_planet->size; zone_id++){

                    working_zone = &working_planet->zones[zone_id];

                    // Compute current population
                    working_zone->population += working_zone->environment * working_zone->food * \
                        (population_growth_factor / working_planet->size);

                    // Compute current infrastructure
                    old_infrastructure = working_zone->infrastructure;
                    working_zone->infrastructure = old_infrastructure + working_zone->material * \
                        sqrt((working_zone->environment * old_infrastructure * working_zone->population) \
                         / working_planet->size) + K_IF / working_planet->size;

                }

            }
        }

        // Recompute population mean for each planet
        for (i = min_height; i < max_height; i++)
            for (j = min_width; j < max_width; j++){
                working_planet = &working_galaxy.planets[get_planet_id(j, i)];
                working_planet->population_mean = compute_planet_population_mean(j, i);
            }

    }

    // Parallel writing of results to file
    if (processes_count > 1)
        MPI_Barrier(MPI_COMM_WORLD);
    for (i = 0; i < working_galaxy.height; i++){
        for (j = 0; j < working_galaxy.width; j++){

            // If the planet is from this process, print its details
            if (i >= min_height && i < max_height && j >= min_width && j < max_width){

                // Open file
                if (output_file == NULL){
                    output_file = fopen(output_filename, "a");
                    RET_CONDITION_CHECKER(output_file == NULL, 1);
                }

                // Write the galaxy dimensions
                if (i == 0 && j == 0){
                    fprintf(output_file, "%d %d\n", working_galaxy.height, working_galaxy.width);
                    fflush(output_file);
                }

                // Write current planet size and its zones
                working_planet = &working_galaxy.planets[get_planet_id(j, i)];
                fprintf(output_file, "%d\n", working_planet->size);
                fflush(output_file);
                for (zone_id = 0; zone_id < working_planet->size; zone_id++){
                    working_zone = &working_planet->zones[zone_id];
                    fprintf(output_file, "%f %f %f %f %f\n", \
                        working_zone->food, working_zone->material, \
                        working_zone->environment, working_zone->population, working_zone->infrastructure);
                    fflush(output_file);
                }

            }
            else{

                // Close file if the current planet isn't of this process
                if (output_file != NULL){
                    fclose(output_file);
                    output_file = NULL;
                }

            }

            if (processes_count > 1)
                MPI_Barrier(MPI_COMM_WORLD);

        }
    }

    MAIN_EXIT:
        MPI_Finalize();
        return 0;

}
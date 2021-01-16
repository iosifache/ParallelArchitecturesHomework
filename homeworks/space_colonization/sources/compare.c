/*
 * Modified version of the program used to compare files containing floating
 * point values
 *
 * Original sourc author: Bureaca Emil
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_ERROR 0.001
#define MAX_RELATIVE_ERROR 20
#define SUMMARY_MODE 1

#define MAX(a,b) ((a) > (b) ? (a) : (b))

int main(int argc, char **argv){

    FILE *first_file, *second_file;
    double max_error = 0, max_error_a = 0, max_error_b = 0, \
        max_relative_error = 0, max_relative_error_a = 0, max_relative_error_b = 0, \
        a, b, error, relative_error;
    int errors_count = 0, relative_errors_count = 0;

    if(argc < 3)
        goto DIFFERS;

    first_file = fopen(argv[1], "rt");
    second_file = fopen(argv[2], "rt");
    if(first_file == NULL || second_file == NULL)
        goto NOT_EXISTS;

    while(fscanf(first_file, "%lf", &a) != EOF){

        // If the second file is shorter than the first one
        if(fscanf(second_file,"%lf", &b) == EOF)
            goto DIFFERS;

        // Process the error
        error = fabs(a - b);
        relative_error = (error * 100) / MAX(a, b);
        if(error > MAX_ERROR){
            errors_count++;
            if (SUMMARY_MODE){
                if (error > max_error){
                    max_error_a = a;
                    max_error_b = b;
                    max_error = error;
                }
            }
            else
                printf("[!] Different pair (%lf != %lf) with error of %lf.\n", a, b, error);
        }
        if(a > 1 && relative_error > MAX_RELATIVE_ERROR){
            relative_errors_count++;
            if (SUMMARY_MODE){
                if (relative_error > max_relative_error){
                    max_relative_error_a = a;
                    max_relative_error_b = b;
                    max_relative_error = relative_error;
                }
            }
            else
                printf("[!] Different pair (%lf != %lf) with relative error of %lf%%.\n", a, b, relative_error);
        }

    }

    // If the second file is longer than the first one
    if (fscanf(second_file,"%lf", &b) != EOF)
        goto DIFFERS;

    // If there are errors
    if (errors_count > 0 || relative_errors_count > 0){
        if (SUMMARY_MODE)
            printf("[!] Summary of errors greater than %lf in value and %d%% relative:\n\
            \t- number of errors: %d\n\
            \t- maximum error: %lf (|%lf - %lf|)\n\
            \t- number of relative errors: %d\n\
            \t- maximum relative error: %lf%% (|%lf - %lf|)\n"\
            , MAX_ERROR, MAX_RELATIVE_ERROR, errors_count, max_error, max_error_a, max_error_b, \
            relative_errors_count, max_relative_error, max_relative_error_a, max_relative_error_b);
        goto DIFFERS;
    }

    // If files are identical
    printf("[+] Files seems identical!\n");
    return 0;

    NOT_EXISTS:
        printf("[!] Files not exists!\n");
        return 1;
    DIFFERS:
        printf("[!] Files differs!\n");
        return 1;

}
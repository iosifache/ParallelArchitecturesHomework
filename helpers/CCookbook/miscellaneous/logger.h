#ifndef _LOGGER_H_

#define _LOGGER_H_

#include <stdio.h>

/*
 * If a variable is set, prints a new line terminated, formatted string
 */
#define LOGGER(enable, format, ...) \
    if (enable){ \
        printf(format "\n", __VA_ARGS__); \
        fflush(stdout); \
    }

/*
 * If a variable is set, prints a double array, element by element and prefixed
 * by a label
 */
#define LOG_DOUBLE_ARRAY(enable, array, size, label_format, ...) \
    if (enable) {\
        printf(label_format "\n" __VA_OPT__(,) __VA_ARGS__); \
        for (int counter = 0; counter < size; counter++) \
            printf("%lf ", array[counter]); \
        printf("\n"); \
        fflush(stdout); \
    };

#endif
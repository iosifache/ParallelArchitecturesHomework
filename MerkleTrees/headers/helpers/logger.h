#ifndef _LOGGER_H_

#define _LOGGER_H_

#include <stdio.h>

#define LOGGER(enable, format, ...) \
    if (enable) printf(format "\n", __VA_ARGS__);

#endif
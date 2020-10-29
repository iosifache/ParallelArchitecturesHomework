#ifndef _STRING_OPERATIONS_H

#define _STRING_OPERATIONS_H

/*
 *  djb2 hash function, created by Dan Bernstei
 */
unsigned long djb2(char *string){

    unsigned long hash = 5381;
    int c;

    while ((c = *string++))
        hash = ((hash << 5) + hash) + c;

    return hash;

}

#endif
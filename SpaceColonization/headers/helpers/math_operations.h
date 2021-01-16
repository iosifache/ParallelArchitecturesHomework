#ifndef _MATH_OPERATIONS_H

#define _MATH_OPERATIONS_H

inline int min(int first, int second){

    if (first < second)
        return first;

    return second;

}

inline int int_sqrt(int x){

    int i, j;

    if (x < 2){
        x = -(x > 0) & x;
    }
    else{
        i = int_sqrt(x >> 2) << 1;
        j = i + 1;
        x = ((j * j) > x) ? i : j;
    }

    return x;

}

inline int gcd(int a, int b){

    return b == 0 ? a : gcd(b, a % b);

}

#endif
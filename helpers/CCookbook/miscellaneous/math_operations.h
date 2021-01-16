#ifndef _MATH_OPERATIONS_H

#define _MATH_OPERATIONS_H

/*
 * Return the minimum of two integer variables
 */
int min(int first, int second){

    if (first < second)
        return first;

    return second;

}

/*
 * Computes the integer root square of a number
 * 
 * Source: https://gist.github.com/hamsham/380992a13351c5efef63
 */
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

/*
 * Computes the great common divisors of two numbers
 */
inline int gcd(int a, int b){

    return b == 0 ? a : gcd(b, a % b);

}

#endif
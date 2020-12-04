/*
 * Author: Cristian Chilipirea
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define MAX_CHARS 300
#define NUM_LETTERS_IN_ALPHABET 26

int *availableNumbers;
int N;
int seed;
int difficulty;
int transmitterThreads;
int numbersLeft;

void getArgs(int argc, char **argv){

    int i;

    if(argc < 5){
        printf("Not enough paramters: ./program N transmitterThreads difficulty seed\n");
        exit(1);
    }

    N = atoi(argv[1]);
    transmitterThreads = atoi(argv[2]);
    difficulty = atoi(argv[3]);
    seed = atoi(argv[4]);

    numbersLeft = N;
    availableNumbers = malloc((N + 1) * sizeof(int));
    for(i = 0; i < N; i++)
        availableNumbers[i] = i;

}

char *generateString(){

    char *string;
    int numChars;

    numChars = 1 + (rand() % MAX_CHARS);
    string = malloc((numChars + 1) * sizeof(char));
    for(int i = 0; i < numChars; i++){
        string[i] = 'a' + (rand() % NUM_LETTERS_IN_ALPHABET);
    }
    string[numChars]='\0';

    return string;

}

int getUniqueNum(){

    int index, retValue, i;

    index = rand() % numbersLeft;
    retValue = availableNumbers[index];
    numbersLeft--;
    for(i = index; i < numbersLeft; i++)
        availableNumbers[i] = availableNumbers[i + 1];

    return retValue;

}

int main(int argc, char **argv){

    getArgs(argc, argv);

    srand(seed);

    printf("%i %i %i\n", N, transmitterThreads, difficulty);
    for(int i = 0; i < N; i++){
        printf("%i %s\n", getUniqueNum(), generateString());
    }

    return 0;

}
#ifndef _CHARACTERS_H

#define _CHARACTERS_H

/*
 * Gets letter (uppercase or lowercase) index in the alphabet
 */
#define GET_LETTER_INDEX(character) \
    (character >= 'a' && character <= 'z') ? (character - 'a') : \
        (character >= 'A' && character <= 'Z') ? (character + 40 - 'a') : -1

/*
 * Converts an uppercase letter into a lowercase one
 */
#define GET_LOWERCASE_LETTER_INDEX(letter) letter - 'a'

/*
 * Checks if a character is a lowercase letter
 */
#define CHECK_IF_LOWERCASE_LETTER(character) \
    (character >= 'a' && character <= 'z')

/*
 * Checks if a character is a number
 */
#define CHECK_IF_NUMBER(character) \
    (character >= '0' && character <= '9')

#endif
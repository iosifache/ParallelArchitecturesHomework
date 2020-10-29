#ifndef _CHAR_OPERATIONS_H

#define _CHAR_OPERATIONS_H

#define GET_LETTER_INDEX(character) \
    (character >= 'a' && character <= 'z') ? \
        (character - 'a') : \
        (character >= 'A' && character <= 'Z') ? \
            (character + 40 - 'a') : -1

#define GET_LOWERCASE_LETTER_INDEX(letter) letter - 'a'

#define CHECK_IF_LOWERCASE_LETTER(character) \
    (character >= 'a' && character <= 'z')

#endif
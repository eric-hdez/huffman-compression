#include "code.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initializers --------------------------------------------------------------------------

// intializes a Code Structure on the Stack
//
Code code_init() {
    Code C;

    C.top = 0;
    memset(C.bits, 0, MAX_CODE_SIZE);

    return C;
}

// Accessor Functions --------------------------------------------------------------------

// returns the total size of the Code
//
uint32_t code_size(Code *C) {
    return C ? C->top : 0;
}

// returns true if a Code is empty, false otherwise
//
bool code_empty(Code *C) {
    return C && (C->top == 0) ? true : false;
}

// returns true if a Code is full, false otherwise
//
bool code_full(Code *C) {
    return C && (C->top == ALPHABET) ? true : false;
}

// Operation Functions -------------------------------------------------------------------

// pushes a bit on to the Code, returning true if successful,
// false if Code is full
//
bool code_push_bit(Code *C, uint8_t bit) {
    if (code_full(C)) {
        return false;
    }

    if (bit) {
        set_bit(C->bits, C->top);
    } else {
        clr_bit(C->bits, C->top);
    }

    C->top += 1;

    return true;
}

// pops a bit off the Code, returning true if successful,
// false if Code is empty
//
bool code_pop_bit(Code *C, uint8_t *bit) {
    if (code_empty(C)) {
        return false;
    }

    C->top -= 1;
    *bit = get_bit(C->bits, C->top);

    return true;
}

// Other Functions -----------------------------------------------------------------------

// prints a Code for debugging purposes
//
void code_print(Code *C) {
    for (uint32_t i = 0; i < code_size(C); i++) {
        printf("%d ", get_bit(C->bits, i));
    }
}

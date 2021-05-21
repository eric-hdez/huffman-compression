#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

// Struct Defn ---------------------------------------------------------------------------

struct Stack {
    uint32_t top;
    uint32_t capacity;
    Node **items;
};

// Constructors & Destructors ------------------------------------------------------------

// constructor for a Stack ADT
//
Stack *stack_create(uint32_t capacity) {
    Stack *S = (Stack *) malloc(sizeof(Stack));
    if (!S) {
        return (Stack *) 0;
    }

    S->top = 0;
    S->capacity = capacity;

    S->items = (Node **) calloc(capacity, sizeof(Node *));
    if (!S->items) {
        free(S);
        S = NULL;
        return (Stack *) 0;
    }

    return S;
}

// destructor for a Stack ADT
//
void stack_delete(Stack **pS) {
    if (pS && *pS) {
        if ((*pS)->items) {
            free((*pS)->items);
        }
    }

    free(*pS);
    *pS = NULL;
}

// Accessor Functions --------------------------------------------------------------------

// returns true if a Stack is empty, false otherwise
//
bool stack_empty(Stack *S) {
    return S && (S->top == 0) ? true : false;
}

// returns true if a stack is full, false otherwise
//
bool stack_full(Stack *S) {
    return S && (S->top == S->capacity) ? true : false;
}

// returns the size of a Stack
//
uint32_t stack_size(Stack *S) {
    return S ? S->top : 0;
}

// Operations & Procedures ---------------------------------------------------------------

// pushes a Node pointer onto a Stack
//
bool stack_push(Stack *S, Node *N) {
    if (stack_full(S)) {
        return false;
    }

    S->items[S->top] = N;
    S->top += 1;

    return true;
}

// pops a Node pointer off a Stack
//
bool stack_pop(Stack *S, Node **N) {
    if (stack_empty(S)) {
        return false;
    }

    S->top -= 1;
    *N = S->items[S->top];

    return true;
}

// Other Functions -----------------------------------------------------------------------

// prints a Stack for debugging purposes
//
void stack_print(Stack *S) {
    printf("----\n");
    for (uint32_t i = 0; i < S->top; i++) {
        node_print(S->items[i]);
    }
    printf("----\n");
}

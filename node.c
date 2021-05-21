#include "node.h"

#include <stdio.h>
#include <stdlib.h>

// Constructor for a Node Object (Huffman Tree)
//
Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *N = malloc(sizeof(Node));
    if (!N) {
        return (Node *) 0;
    }

    N->symbol = symbol;
    N->frequency = frequency;
    N->left = N->right = NULL;

    return N;
}

// Destructor for a Node Object (Huffman Tree)
//
void node_delete(Node **pN) {
    if (pN && *pN) {
        free(*pN);
        *pN = NULL;
    }
}

// joins two nodes, left and right, with a parent node w/
// sym -> '$' and frequency left->freq + right->freq
//
Node *node_join(Node *left, Node *right) {
    Node *J = node_create('$', left->frequency + right->frequency);

    J->left = left;
    J->right = right;

    return J;
}

// printing function for a Huffman Tree
//
void node_print(Node *N) {
    if (N) {
        node_print(N->left);
        printf("%c ", N->symbol);
        node_print(N->right);
    }
}

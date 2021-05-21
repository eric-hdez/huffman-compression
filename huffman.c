#include "huffman.h"

#include "pq.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

// Given a histogram of ASCII codes 0-255, constructs a Huffman tree
// and returns the root Node
//
Node *build_tree(uint64_t hist[static ALPHABET]) {
    PriorityQueue *PQ = pq_create(ALPHABET);
    Node *Root; // Root Node for return

    // build PriorityQueue from sym histogram
    for (int sym = 0; sym < ALPHABET; sym++) {
        if (hist[sym] > 0) {
            Node *N = node_create(sym, hist[sym]);
            enqueue(PQ, N);
        }
    }

    // build Huffman tree from the PriorityQueue
    while (pq_size(PQ) > 1) {
        Node *Left, *Right, *Join;

        dequeue(PQ, &Left);
        dequeue(PQ, &Right);

        Join = node_join(Left, Right);
        enqueue(PQ, Join);
    }

    // dequeue and return the Huffman tree root
    dequeue(PQ, &Root);
    pq_delete(&PQ);

    return Root;
}

// subroutine for build_codes() that builds a unique code copy for each
// leaf in the Huffman tree
//
void build(Node *root, Code table[static ALPHABET], Code C) {
    uint8_t discard; // just to discard the pops

    // base case, Node is a leaf
    if (!root->left && !root->right) {
        table[root->symbol] = C;
        return;
    }

    // going down the left side, push a 0 to code
    if (root->left) {
        code_push_bit(&C, 0);
        build(root->left, table, C);
        code_pop_bit(&C, &discard);
    }

    // going down the right side, push a 1 to code
    if (root->right) {
        code_push_bit(&C, 1);
        build(root->right, table, C);
        code_pop_bit(&C, &discard);
    }
}

// builds the code for each leaf node in a Huffman tree
//
void build_codes(Node *root, Code table[static ALPHABET]) {
    Code C = code_init();
    build(root, table, C);
}

// given a post order Huffman tree dump, reconstructes the Huffman tree and
// and returns the root Node
//
Node *rebuild_tree(uint16_t nbytes, uint8_t tree_dump[static nbytes]) {
    Stack *S = stack_create(nbytes);
    Node *Root; // Root Node for return

    for (uint16_t sym = 0; sym < nbytes; sym++) {
        if (tree_dump[sym] == 'L') {
            Node *N = node_create(tree_dump[sym + 1], 1);
            stack_push(S, N);
            sym++; // skip to prevent repeat if sym was an 'L'
            continue;
        }

        if (tree_dump[sym] == 'I') {
            Node *Right, *Left, *Join;

            stack_pop(S, &Right);
            stack_pop(S, &Left);

            Join = node_join(Left, Right);
            stack_push(S, Join);
        }
    }

    // dequeue and return the root of reconstructed Huffman tree
    stack_pop(S, &Root);
    stack_delete(&S);

    return Root;
}

// destructor for a Huffman tree
//
void delete_tree(Node **root) {
    if (root && *root) {
        if ((*root)->left) {
            delete_tree(&(*root)->left);
        }

        if ((*root)->right) {
            delete_tree(&(*root)->right);
        }

        node_delete(root);
    }
}

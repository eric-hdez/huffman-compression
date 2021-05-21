#include "pq.h"

#include <stdio.h>
#include <stdlib.h>

#define ROOT 1 // root index, 0 reserved to denote NULL children / no parent

// Struct & Members Defn -----------------------------------------------------------------

struct PriorityQueue {
    uint32_t capacity;
    uint32_t size;
    Node **heap;
};

// Helper Functions ----------------------------------------------------------------------

// swaps two Nodes with each other in a min-heap
//
static inline void swap(Node **A, Node **B) {
    Node *temp = *A;
    *A = *B;
    *B = temp;
}

// Constructors / Destructors ------------------------------------------------------------

// Constructor for a Priority Queue, using an underlying min-heap
//
PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *PQ = malloc(sizeof(PriorityQueue));
    if (!PQ) {
        return (PriorityQueue *) 0;
    }

    PQ->heap = calloc(capacity + 1, sizeof(Node *));
    if (!PQ->heap) {
        free(PQ);
        PQ = NULL;
        return (PriorityQueue *) 0;
    }

    PQ->size = 0, PQ->capacity = capacity;

    return PQ;
}

// Destructor for a Priority Queue ADT
//
void pq_delete(PriorityQueue **PQ) {
    if (PQ && *PQ) {
        if ((*PQ)->heap) {
            for (uint32_t n = ROOT; n <= (*PQ)->size; n++) {
                node_delete(&(*PQ)->heap[n]);
            }
        }

        free((*PQ)->heap);
        free(*PQ);

        *PQ = NULL;
    }
}

// Accessor Functions --------------------------------------------------------------------

// returns true if a Priority Queue is empty, false otherwise
//
bool pq_empty(PriorityQueue *PQ) {
    return (PQ && !PQ->size) ? true : false;
}

// returns true if a priority Queue is full, false otherwise
//
bool pq_full(PriorityQueue *PQ) {
    return (PQ && (PQ->size == PQ->capacity)) ? true : false;
}

// returns the number of items in a Priority Queue
//
uint32_t pq_size(PriorityQueue *PQ) {
    return PQ ? PQ->size : 0;
}

// Operation Functions -------------------------------------------------------------------

// SUBROUTINE FOR PRIORITYQUEUE'S ENQUEUE() :
// inserts an element into a min heap by swapping Nodes iteratively
//
void min_heap_insert(PriorityQueue *PQ, Node *N) {
    uint32_t i = PQ->size + 1;
    uint32_t parent = i / 2;

    PQ->heap[i] = N;

    while (i > ROOT && PQ->heap[parent]->frequency > PQ->heap[i]->frequency) {
        swap(&PQ->heap[parent], &PQ->heap[i]);
        i = parent, parent = i / 2;
    }
}

// enqueues a Node onto PriorityQueue, returns true if successful,
// false otherwise
//
bool enqueue(PriorityQueue *PQ, Node *N) {
    if (!PQ || !N || pq_full(PQ)) {
        return false;
    }

    if (pq_empty(PQ)) {
        PQ->heap[ROOT] = N;
    } else {
        min_heap_insert(PQ, N);
    }

    PQ->size += 1;

    return true;
}

// SUBROUTINE FOR PRIORITYQUEUE'S DEQUEUE() :
// fixes a heap, shifting elements down or up the minimum heap
//
void min_heapify(PriorityQueue *PQ, uint32_t i) {
    uint32_t lc = i * 2; // left child of heap[i]'s index
    uint32_t rc = i * 2 + 1; // right child of heap[i]'s index
    uint32_t smallest = 0; // 0 is reserved in the list, denotes nothing

    if (lc <= PQ->size && PQ->heap[lc]->frequency < PQ->heap[i]->frequency) {
        smallest = lc;
    } else {
        smallest = i;
    }

    if (rc <= PQ->size && PQ->heap[rc]->frequency < PQ->heap[smallest]->frequency) {
        smallest = rc;
    }

    if (smallest != i) {
        swap(&PQ->heap[i], &PQ->heap[smallest]);
        min_heapify(PQ, smallest);
    }
}

// dequeue's the Node with highest off of the PriorityQueue
//
bool dequeue(PriorityQueue *PQ, Node **N) {
    if (!PQ || !N || pq_empty(PQ)) {
        return false;
    }

    Node *min = PQ->heap[ROOT];
    PQ->heap[ROOT] = PQ->heap[PQ->size];
    PQ->size -= 1;

    min_heapify(PQ, ROOT);

    *N = min;
    return true;
}

// Other Functions -----------------------------------------------------------------------

// prints a PriorityQueue for debugging purposes (for now, the indices)
//
void pq_print(PriorityQueue *PQ) {
    for (uint32_t n = ROOT; n <= PQ->size; n++) {

        uint32_t left = (n * 2) < PQ->size ? (n * 2) : 0;
        uint32_t right = (n * 2 + 1) < PQ->size ? (n * 2 + 1) : 0;
        uint32_t parent = n / 2;

        printf("%d : {parent: %d, left: %d, right: %d}\n", n, parent, left, right);
    }
}

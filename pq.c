#include "pq.h"

#include <stdio.h>
#include <stdlib.h>

#define NIL        0  // denotes nothing
#define ROOT       1  // denotes the root index
#define parent(i)  i / 2
#define left(i)    i * 2
#define right(i)   i * 2 + 1

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
                if ((*PQ)->heap[n]) {
                    node_delete(&(*PQ)->heap[n]);
                }
                
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
//
// inserts an element into a min heap by swapping Nodes iteratively to
// prevserve min heap properties
//
void min_heap_insert(PriorityQueue *PQ, Node *N) {
    uint32_t i = PQ->size + 1;

    PQ->heap[i] = N;

    while (i > ROOT && PQ->heap[parent(i)]->frequency > PQ->heap[i]->frequency) {
        swap(&PQ->heap[parent(i)], &PQ->heap[i]);
        i = parent(i);
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
// fixes a heap, shifting elements down or up the minimum heap after
// extracting the min-node
//
void min_heapify(PriorityQueue *PQ, uint32_t i) {
    uint32_t lc = left(i);  // left child of heap[i]
    uint32_t rc = right(i);  // right child of heap[i]
    uint32_t smallest = NIL;

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

// dequeue's the Node with highest priority off of a PriorityQueue,
// returns true if successful
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

        uint32_t left = left(n) < PQ->size ? left(n) : 0;
        uint32_t right = right(n) < PQ->size ? right(n) : 0;
        uint32_t parent = parent(n);

        printf("%d : {parent: %d, left: %d, right: %d}\n", n, parent, left, right);
    }
}

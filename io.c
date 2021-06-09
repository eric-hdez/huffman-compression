#include "io.h"

#include "header.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BITS 8  // number of bits in a single byte

uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

static uint8_t bitbuff[BLOCK] = { 0 };
static int bitindex = 0;

static uint8_t codebuff[BLOCK] = { 0 };
static int codeindex = 0;

// performs buffered reads using the read() system call,
// returns number of bytes read
//
int read_bytes(int infile, uint8_t *buf, int nbytes) {
    int reads = -1, total = 0;

    while ((total != nbytes) && (reads != 0)) {
        if ((reads = read(infile, buf, nbytes)) == -1) {
            fprintf(stderr, "[ERR : read_bytes()] could not read from infile\n");
            exit(EXIT_FAILURE);
        }

        total += reads;
        buf += reads;
    }

    bytes_read += total;
    return total;
}

// performs wri)es from buffered input using the write()
// system call, returns number of bytes written
//
int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int writes = -1, total = 0;

    while ((total != nbytes) && (writes != 0)) {
        if ((writes = write(outfile, buf, nbytes)) == -1) {
            fprintf(stderr, "[ERR : write_bytes()] could not write to outfile.\n");
            exit(EXIT_FAILURE);
        }

        total += writes;
        buf += writes;
    }

    bytes_written += total;
    return total;
}

// reads bits from the input file and returns them into *bit
//
bool read_bit(int infile, uint8_t *bit) {
    static int end = -1;

    if (!bitindex) {
        int reads = read_bytes(infile, bitbuff, BLOCK);
        if (reads < BLOCK) {
            end = reads + 1;
        }
    }

    *bit = get_bit(bitbuff, bitindex);
    bitindex = (bitindex + 1) % (BITS * BLOCK);

    return (bitindex == (BITS * end)) ? false : true;
}

// writes a full buffer of Codes out to an output file
//
void write_code(int outfile, Code *C) {
    for (uint32_t bit = 0; bit < code_size(C); bit++) {
        if (get_bit(C->bits, bit)) {
            set_bit(codebuff, codeindex);
        }

        codeindex = (codeindex + 1) % (BITS * BLOCK);

        if (!codeindex) {
            write_bytes(outfile, codebuff, BLOCK);
            memset(codebuff, 0, BLOCK);
        }
    }
}

// flush the remaining bytes containing Codes in the buffers
//
void flush_codes(int outfile) {
    int to_write = !(codeindex % BITS) ? (codeindex / BITS) : (codeindex / BITS) + 1;
    write_bytes(outfile, codebuff, to_write);
}

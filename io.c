#include "io.h"

#include "error.h"
#include "header.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BITS_PER_BYTE 8  // number of bits in a single byte

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
    int rbytes = -1, total = 0;

    while (total != nbytes && rbytes != 0) {
        rbytes = read(infile, buf + total, nbytes - total);
        if (rbytes == -1) {
            WARN("failed to read bytes from the infile");
            continue;
        }

        total += rbytes;
    }

    bytes_read += total;
    return total;
}

// performs writes from buffered input using the write()
// system call, returns number of bytes written
//
int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int wbytes = -1, total = 0;

    while (total != nbytes && wbytes != 0) {
        wbytes = write(outfile, buf + total, nbytes - total);
        if (wbytes == -1) {
            WARN("failed to write bytes to the outfile.\n");
            continue;
        }

        total += wbytes;
    }

    bytes_written += total;
    return total;
}

// reads bits from an input file and returns them into *bit
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
    bitindex = (bitindex + 1) % (BITS_PER_BYTE * BLOCK);

    return bitindex == (BITS_PER_BYTE * end) ? true : false;
}

// writes a full buffer of Codes out to an output file
//
void write_code(int outfile, Code *C) {
    for (uint32_t bit = 0; bit < code_size(C); bit++) {
        if (get_bit(C->bits, bit)) {
            set_bit(codebuff, codeindex);
        }

        codeindex = (codeindex + 1) % (BITS_PER_BYTE * BLOCK);

        if (!codeindex) {
            write_bytes(outfile, codebuff, BLOCK);
            memset(codebuff, 0, BLOCK);
        }
    }
}

// flushes the remaining bytes containing Codes in the buffers
//
void flush_codes(int outfile) {
    write_bytes(outfile, codebuff, bytes(codeindex));
}

#include "header.h"
#include "huffman.h"
#include "io.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

uint8_t buffer[BLOCK] = { 0 };

// creates a histogram of the symbols present in an input file to be compressed
//
void hist_create(int infile, uint64_t hist[ALPHABET], uint16_t *unique_sym) {
    int bytes;
    uint8_t buff[BLOCK] = { 0 };

    while ((bytes = read_bytes(infile, buff, BLOCK)) > 0) {
        for (int sym = 0; sym < bytes; sym++) {
            if (!hist[buff[sym]]) {
                *unique_sym += 1;
            }

            hist[buff[sym]] += 1;
        }
    }
}

// dumps the encoded Huffman tree into the outfile
//
void dump_tree(int outfile, Node *root) {
    // base case for a leaf Node
    if (!root->left && !root->right) {
        uint8_t buff[2] = { 'L', root->symbol };
        write_bytes(outfile, buff, 2);
        return;
    }

    // continue recursing down the Huffman tree
    dump_tree(outfile, root->left);
    dump_tree(outfile, root->right);

    // internal Node handle case
    uint8_t buff[1] = { 'I' };
    write_bytes(outfile, buff, 1);
}

// main function for Huffman encoding
int main(int argc, char *argv[]) {
    int options;

    // default i/o and verbose options
    int in = STDIN_FILENO;
    int out = STDOUT_FILENO;
    bool verbose = false;

    while ((options = getopt(argc, argv, "hi:o:v")) != -1) {
        switch (options) {

        case 'h': {
            fprintf(stdout, "Usage: %s -i <infile> -o <outfile> -v (optional stats)\n", argv[0]);
            exit(EXIT_SUCCESS);

            break;
        }
        case 'i': {
            if ((in = open(optarg, O_RDONLY)) == -1) {
                fprintf(stderr, "%s: failed to open file or file does not exist.\n", optarg);
                close(out);
                exit(EXIT_FAILURE);
            }

            break;
        }
        case 'o': {
            if ((out = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
                fprintf(stderr, "%s: failed to open file or create it.\n", optarg);
                close(in);
                exit(EXIT_FAILURE);
            }

            break;
        }
        case 'v': {
            verbose = true;
            break;
        }
        }
    }

    // construction of the histogram from input file
    uint64_t histogram[ALPHABET] = { 0 };
    uint16_t unique_syms = 0;

    hist_create(in, histogram, &unique_syms);

    unique_syms += !histogram[0] ? 1 : 0;
    unique_syms += !histogram[ALPHABET - 1] ? 1 : 0;
    histogram[0]++, histogram[ALPHABET - 1]++;

    // building the Huffman tree using the histogram
    Node *HuffRoot = build_tree(histogram);

    // build the codes for the Huffman tree
    Code code_table[ALPHABET] = { 0 };
    build_codes(HuffRoot, code_table);

    // grabbing infile permissions and setting them for outfile
    struct stat buff;
    fstat(in, &buff);
    fchmod(out, buff.st_mode);

    // creating the outgoing header for output file
    Header outheader = { 0, 0, 0, 0 };
    outheader.magic = MAGIC;
    outheader.permissions = buff.st_mode;
    outheader.tree_size = 3 * unique_syms - 1;
    outheader.file_size = buff.st_size;

    // writing the header to the output file and dumping the tree
    write_bytes(out, (uint8_t *) &outheader, sizeof(Header));
    dump_tree(out, HuffRoot);

    // now, write out the codes to the output file
    lseek(in, 0, SEEK_SET);
    int bytes = 0;

    while ((bytes = read_bytes(in, buffer, BLOCK)) > 0) {
        for (int sym = 0; sym < bytes; sym++) {
            write_code(out, &code_table[buffer[sym]]);
        }
    }

    // flush any remaining codes out
    flush_codes(out);

    // take the garbage out
    delete_tree(&HuffRoot);
    close(in);
    close(out);

    return 0;
}

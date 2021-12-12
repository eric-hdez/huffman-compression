#include "error.h"
#include "header.h"
#include "huffman.h"
#include "io.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define OPTIONS "hi:o:v"

// creates a histogram of the symbols present in an input file to be compressed
//
void hist_create(int infile, uint64_t hist[ALPHABET], uint16_t *unique_sym) {
    uint8_t buffer[BLOCK] = { 0 };
    ssize_t nbytes = 0;

    while ((nbytes = read_bytes(infile, buffer, BLOCK)) > 0) {
        for (int sym = 0; sym < nbytes; sym++) {
            if (!hist[buffer[sym]]) {
                *unique_sym += 1;
            }

            hist[buffer[sym]] += 1;
        }
    }
}

// dumps the encoded Huffman tree into the outfile
//
void dump_tree(int outfile, Node *root) {
    // base case for a leaf node
    if (is_leaf_node(root)) {
        uint8_t buffer[] = { 'L', root->symbol };
        write_bytes(outfile, buffer, 2);
        return;
    }

    // continue recursing down the Huffman tree
    dump_tree(outfile, root->left);
    dump_tree(outfile, root->right);

    // internal Node handle case
    uint8_t buffer[] = { 'I' };
    write_bytes(outfile, buffer, 1);
}

// prints the program usage menu
//
void print_usage(FILE *stream, char *bin) {
    fprintf(stream, 
        "USAGE\n"
        "  %s [-h] [-i infile] [-o outfile]\n"
        "\n"
        "OPTIONS\n"
        "  -h             Program help menu\n"
        "  -v             Print compression statisitcs\n"
        "  -i infile      Input file for compression\n"
        "  -o outfile     Output file for compressed file\n",
        bin);
}

// main function for Huffman encoding
int main(int argc, char *argv[]) {
    int options;

    // default i/o and verbose options
    int fdin = STDIN_FILENO;
    int fdout = STDOUT_FILENO;
    bool verbose = false;

    while ((options = getopt(argc, argv, OPTIONS)) != -1) {
        switch (options) {
        case 'h': print_usage(stdout, argv[0]); break;
        case 'i': 
            if ((fdin = open(optarg, O_RDONLY)) == -1) {
                close(fdout);
                ERROR(EXIT_FAILURE, "failed to open infile for reading");
            } 
            break;
        case 'o': 
            if ((fdout = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
                close(fdin);
                ERROR(EXIT_FAILURE, "failed to open outfile for writing");
            }; 
            break;
        case 'v': verbose = true; break;
        default: print_usage(stderr, argv[0]); break;
        }
    }

    // construction of the histogram from input file
    uint64_t histogram[ALPHABET] = { 0 };
    uint16_t unique_syms = 0;

    hist_create(fdin, histogram, &unique_syms);

    unique_syms += !histogram[0] ? 1 : 0;
    unique_syms += !histogram[ALPHABET - 1] ? 1 : 0;
    histogram[0]++, histogram[ALPHABET - 1]++;

    // building the Huffman tree and its codes
    Node *HuffRoot = build_tree(histogram);

    Code code_table[ALPHABET] = { 0 };
    build_codes(HuffRoot, code_table);

    // grabbing infile permissions and setting them for outfile
    struct stat buff;
    fstat(fdin, &buff);
    fchmod(fdout, buff.st_mode);

    // creating the outgoing header for output file 
    Header outheader = {
        .magic = MAGIC,
        .permissions = buff.st_mode,
        .tree_size = 3 * unique_syms - 1,
        .file_size = buff.st_size
    };

    // writing the header and dumping tree to output file
    write_bytes(fdout, (uint8_t *) &outheader, sizeof(Header));
    dump_tree(fdout, HuffRoot);

    // now, write out the codes to the output file
    lseek(fdin, 0, SEEK_SET);
    uint8_t buffer[BLOCK] = { 0 };
    ssize_t bytes = 0;

    while ((bytes = read_bytes(fdin, buffer, BLOCK)) > 0) {
        for (int sym = 0; sym < bytes; sym++) {
            write_code(fdout, &code_table[buffer[sym]]);
        }
    }

    // flush any remaining codes out
    flush_codes(fdout);

    // print stats if verbose is specified
    if (verbose) {
        double savings = 1.0 - ((double) bytes_written / (double) bytes_read);
        fprintf(stdout, "Uncompressed file size: %" PRIu64 " bytes\n", bytes_read);
        fprintf(stdout, "Compressed file size  : %" PRIu64 " bytes\n", bytes_written);
        fprintf(stdout, "Space saving          : %.2lf%%\n", savings);
    }

    // take the garbage out
    delete_tree(&HuffRoot);
    close(fdin);
    close(fdout);

    return 0;
}

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

// prints the program usage menu
//
void print_usage(FILE *stream, char *bin) {
    fprintf(stream, 
        "USAGE\n"
        "  %s [-h] [-i infile] [-o outfile]\n"
        "\n"
        "OPTIONS\n"
        "  -h             Program help menu\n"
        "  -v             Print decompression statisitcs\n"
        "  -i infile      Input file for decompression\n"
        "  -o outfile     Output file for decompressed file\n",
        bin);
}

int main(int argc, char *argv[]) {
    int option;

    // default i/o and verbose options
    int fdin = STDIN_FILENO;
    int fdout = STDOUT_FILENO;
    bool verbose = false;

    while ((option = getopt(argc, argv, "hi:o:v")) != -1) {
        switch (option) {
        case 'h': print_usage(stderr, argv[0]); break;
        case 'i': {
            if ((fdin = open(optarg, O_RDONLY)) == -1) {
                close(fdout);
                ERROR(EXIT_FAILURE, "failed to open infile for reading");
            }
            break;
        }
        case 'o': {
            if ((fdout = open(optarg, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
                close(fdin);
                ERROR(EXIT_FAILURE, "failed to open outfile for writing");
            }
            break;
        }
        case 'v': verbose = true; break;
        default: print_usage(stderr, argv[0]); break;
        }
    }

    // create a header and read in the compressed file's header
    Header inheader = { 0 };
    read_bytes(fdin, (uint8_t *) &inheader, sizeof(Header));

    // if the MAGIC matches, then the file was compressed using Huffman
    if (inheader.magic != MAGIC) {
        close(fdin);
        close(fdout);
        ERROR(EXIT_FAILURE, "infile magic is invalid, file could not be decompressed");
    }

    fchmod(fdout, inheader.permissions);

    // retrieve the tree dump from compressed file and rebuild the tree
    uint8_t tree_retrieve[inheader.tree_size];
    read_bytes(fdin, tree_retrieve, inheader.tree_size);

    Node *HuffRoot = rebuild_tree(inheader.tree_size, tree_retrieve);

    // buffer and variable setups for reconstructing the original file
    uint8_t buffer[BLOCK];
    Node *curr = HuffRoot;
    uint64_t decodes = 0;
    uint8_t bit;

    // traverse the tree via codes and reconstruct the original file
    while (decodes < inheader.file_size) {
        if (!curr->left && !curr->right) {
            buffer[decodes % BLOCK] = curr->symbol;
            decodes += 1, curr = HuffRoot;

            // flush out buffered decoded syms when full
            if (!(decodes % BLOCK)) {
                write_bytes(fdout, buffer, BLOCK);
            }
        }

        read_bit(fdin, &bit); // read the next bit of codes to traverse tree

        if (!bit && curr->left) {
            curr = curr->left;
        } else if (bit && curr->right) {
            curr = curr->right;
        }
    }

    // flush out the remainder of the buffered syms
    write_bytes(fdout, buffer, decodes % BLOCK);

    // print statistics if verbose enabled
    if (verbose) {
        double savings = 1.0 - ((double) bytes_read / (double) bytes_written);
        fprintf(stdout, "Compressed file size  : %" PRIu64 " bytes\n", bytes_read);
        fprintf(stdout, "Uncompressed file size: %" PRIu64 " bytes\n", bytes_written);
        fprintf(stdout, "Space saving          : %.2lf%%\n", savings);
    }

    // garbage collection time
    delete_tree(&HuffRoot);
    close(fdin);
    close(fdout);

    return 0;
}

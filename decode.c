#include "header.h"
#include "huffman.h"
#include "io.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

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

    // create a header and read in the compressed file's header
    Header inheader = { 0, 0, 0, 0 };
    read_bytes(in, (uint8_t *) &inheader, sizeof(Header));

    // if the MAGIC matches, then the file was compressed using Huffman
    if (inheader.magic != MAGIC) {
        fprintf(stderr, "infile magic invalid, file could not be compressed.\n");
        close(in);
        close(out);
        exit(EXIT_FAILURE);
    }

    fchmod(out, inheader.permissions);

    // retrieve the tree dump from compressed file and rebuild the tree
    uint8_t tree_retrieve[inheader.tree_size];
    read_bytes(in, tree_retrieve, inheader.tree_size);

    Node *HuffRoot = rebuild_tree(inheader.tree_size, tree_retrieve);

    // buffer and variable setups for reconstructing the original file
    uint8_t outbuff[BLOCK];
    Node *curr = HuffRoot;
    uint64_t decodes = 0;
    uint8_t bit;

    // traverse the tree via codes and reconstruct the original file
    while (decodes < inheader.file_size) {
        if (!curr->left && !curr->right) {
            outbuff[decodes % BLOCK] = curr->symbol;
            decodes += 1, curr = HuffRoot;

            // flush out buffered decoded syms when full
            if (!(decodes % BLOCK)) {
                write_bytes(out, outbuff, BLOCK);
            }
        }

        read_bit(in, &bit); // read the next bit of codes to traverse tree

        if (!bit && curr->left) {
            curr = curr->left;
        } else if (bit && curr->right) {
            curr = curr->right;
        }
    }

    // flush out the remainder of the buffered syms
    write_bytes(out, outbuff, decodes % BLOCK);

    // garbage collection time
    delete_tree(&HuffRoot);
    close(in);
    close(out);

    return 0;
}

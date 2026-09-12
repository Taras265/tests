#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printFileContent(const char *filename, int step) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("Error: couldnt open file \"%s\"\n\n", filename);
        return;
    }

    printf("\nFILE: %s\n", filename);

    int c;
    long count = 0;

    while ((c = fgetc(fp)) != EOF) {
        count++;
        if (step <= 1) {
            putchar(c);
        } else if (count % step == 0) {
            putchar(c);
        }
    }

    putchar('\n');
    printf("\n");

    fclose(fp);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Error: no one file was entered\n");
        exit(EXIT_FAILURE);
    }

    int step = 1;
    int start = 1;

    if (strcmp(argv[1], "-Z") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: after flag -Z the integer number needed\n");
            exit(EXIT_FAILURE);
        }

        step = atoi(argv[2]);
        if (step <= 0) {
            fprintf(stderr, "Error: after flag -Z the integer number needed\n");
            exit(EXIT_FAILURE);
        }

        start = 3;
    }

    if (start >= argc) {
        fprintf(stderr, "Error: no one file to processing entered\n");
        exit(EXIT_FAILURE);
    }

    for (int i = start; i < argc; i++) {
        printFileContent(argv[i], step);
    }

    return 0;
}
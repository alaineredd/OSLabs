#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define BUFFERSIZE 256

int main(int argc, char *argv[]) { // argv [0] - child, [1] - filename
    char buffer[BUFFERSIZE];
    FILE* file;

    if (argc != 2) {
    fprintf(stderr, "Usage: %s filename\n", argv[0]);
    exit(1);
}

    file = fopen(argv[1], "w");
    if(file == NULL) {
        perror("file trouble");
        exit(1);
    }
    while(fgets(buffer, BUFFERSIZE, stdin) != NULL) {
        if(isupper(buffer[0])) {
            fprintf(file, "%s", buffer);
        } else {
            printf("line %s is not valid\n", buffer);
        }
    }
    fclose(file);
}
#include "os.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 256

typedef struct {
    char data[BUFFER_SIZE];
    int data_ready;
    int stop;
} shared_data_t;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "fail" << argv[0] << "\n";
        exit(1);
    }
    void* shared_addr = OpenMappedFile("/tmp/shared_memory", sizeof(shared_data_t));
    if (shared_addr == nullptr) {
        perror("OpenMappedFile error");
        exit(1);
    }
    shared_data_t* shared_data = (shared_data_t*)shared_addr;
    const char* output_filename = argv[1];
    while (1) {
        while (shared_data->data_ready == 0 && !shared_data->stop) {
            usleep(1000);
        }
        if (shared_data->stop) {
            break;
        }
        char* line = shared_data->data;
        FILE* output_file = fopen(output_filename, "a");
        if (output_file == NULL) {
            perror("fopen error");
            break;
        }
        if (isupper((unsigned char)line[0])) {
            fprintf(output_file, "%s", line);
            fflush(output_file);
        } else {
            std::cout << "Строка не начинается с заглавной буквы: " << line << "\n";
        }

        fclose(output_file);
        shared_data->data_ready = 0;
    }

    CloseMappedFile((mapped_file_t){shared_addr, sizeof(shared_data_t)});
    return 0;
}
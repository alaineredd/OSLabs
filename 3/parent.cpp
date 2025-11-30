#include "os.h"

#include <signal.h>
#include <iostream>

#include "os.h"

#define BUFFER_SIZE 256
#define SHARED_MEMORY_SIZE 4096

typedef struct {
    char data[BUFFER_SIZE];
    int data_ready;
    int stop;
} shared_data_t;

int main() {
    pid_t pid;
    char filename[BUFFER_SIZE];
    mapped_file_t shared_mem;

    printf("Введите имя файла для результатов: ");
    if (!fgets(filename, BUFFER_SIZE, stdin)) {
        perror("fgets error");
        exit(1);
    }
    filename[strcspn(filename, "\n")] = 0;

    shared_mem = CreateMappedFile("/tmp/shared_memory", sizeof(shared_data_t));
    if (shared_mem.address == NULL) {
        perror("CreateMappedFile error");
        exit(1);
    }

    shared_data_t* shared_data = (shared_data_t*)shared_mem.address;
    memset(shared_data, 0, sizeof(shared_data_t));

    pid = CloneProcess();
    if (pid == -1) {
        perror("fork error");
        CloseMappedFile(shared_mem);
        exit(1);
    }

    if(pid == 0) {
        std::cout << "child proc\n";
        CloseMappedFile(shared_mem);
        char* argv[] = {"child", filename, NULL};
        Exec("./child", argv);
        perror("exec error");
        exit(1);
    } else {
        std::cout << "parent proc\n";
        char buffer[BUFFER_SIZE];
        FILE* output_file = fopen(filename, "w");
        if (output_file == NULL) {
            perror("fopen error");
            CloseMappedFile(shared_mem);
            exit(1);
        }
        while (1) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                break;
            }
            strncpy(shared_data->data, buffer, BUFFER_SIZE - 1);
            shared_data->data_ready = 1;
            while (shared_data->data_ready == 1) {
                usleep(1000);
            }

            if (shared_data->stop) {
                break;
            }
        }
        shared_data->stop = 1;
        shared_data->data_ready = 1;

        fclose(output_file);
        CloseMappedFile(shared_mem);
        WaitProcess();
        printf("Родительский процесс завершен.\n");
    }
}
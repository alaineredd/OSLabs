#include "os.h"

#include <stdio.h>

#define BUFFERSIZE 256

int main() {
    int pipe1[2];
    int pipe2[2];
    pid_t pid;
    char filename[BUFFERSIZE];

    printf("Введите имя файла");
    if(!fgets(filename, BUFFERSIZE, stdin)) {
        perror("fgets error");
        exit(1);
    }

    filename[strcspn(filename, "\n")] = 0;

    if(CreatePipe(pipe1) == -1 || CreatePipe(pipe2) == -1) {
        perror("pipe error");
        exit(1);
    }

    pid = CloneProcess();
    if(pid == -1) {
        perror("fork error");
        exit(1);
    }

    if(pid == 0) {
        // 0r 1w
        ClosePipe(pipe1[1]);
        ClosePipe(pipe2[0]);
        LinkFDtoIN(pipe1[0]);
        ClosePipe(pipe1[0]);
        LinkFDtoOUT(pipe2[1]);
        ClosePipe(pipe2[1]);

        Exec("./child", (char*[]){"child", filename, NULL});
        perror("execl error");
        exit(1);
    } else {
        ClosePipe(pipe1[0]);
        ClosePipe(pipe2[1]);
        char buffer[BUFFERSIZE];
        while(fgets(buffer, BUFFERSIZE, stdin)) {
            WritePipe(pipe1[1], buffer, strlen(buffer));
        }
        ClosePipe(pipe1[1]);

        int bytes;
        while((bytes = read(pipe2[0], buffer, BUFFERSIZE)) > 0) {
            fwrite(buffer, 1, bytes, stdout);
        }
        if(bytes < 0) {
            perror("can't read from pipe2");
        }
        ClosePipe(pipe2[0]);
        wait(NULL);
    }
}
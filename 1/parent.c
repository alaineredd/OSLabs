#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFERSIZE 256

int main() {
    int pipe1[2];
    int pipe2[2];
    pid_t pid;
    char filename[BUFFERSIZE];
    FILE *file;

    printf("Введите имя файла");
    if(!fgets(filename, BUFFERSIZE, stdin)) {
        perror("fgets error");
        exit(1);
    }

    filename[strcspn(filename, "\n")] = 0;

    if(pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe error");
        exit(1);
    }

    pid = fork();
    if(pid == -1) {
        perror("fork error");
        exit(1);
    }

    if(pid == 0) {
        // 0r 1w
        close(pipe1[1]);
        close(pipe2[0]);
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);

        execl("./child", "child", filename, (char*)NULL);
        perror("execl error");
        exit(1);
    } else {
        close(pipe1[0]);
        close(pipe2[1]);
        char buffer[BUFFERSIZE];
        while(fgets(buffer, BUFFERSIZE, stdin)) {
            write(pipe1[1], buffer, strlen(buffer));
        }
        close(pipe1[1]);

        int bytes;
        while((bytes = read(pipe2[0], buffer, BUFFERSIZE)) > 0) {
            fwrite(buffer, 1, bytes, stdout);
        }
        if(bytes < 0) {
            perror("can't read from pipe2");
        }
        close(pipe2[0]);
        wait(NULL);
    }
}
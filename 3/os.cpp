#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t CloneProcess() {
    return fork();
}

int Exec(const char* path, char* argv[]) {
    return execv(path, argv);
}

int WaitProcess() {
    return wait(NULL);
}

mapped_file_t CreateMappedFile(const char* filename, size_t size) {
    mapped_file_t result = {NULL, 0};
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open file error");
        return result;
    }
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate error");
        close(fd);
        return result;
    }
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap error");
        close(fd);
        return result;
    }
    close(fd);
    result.address = addr;
    result.size = size;
    return result;
}

void* OpenMappedFile(const char* filename, size_t size) {
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open file error");
        return NULL;
    }
    
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    
    if (addr == MAP_FAILED) {
        perror("mmap error");
        return NULL;
    }
    
    return addr;
}

int CloseMappedFile(mapped_file_t mf) {
    if (mf.address && mf.size > 0) {
        return munmap(mf.address, mf.size);
    }
    return -1;
}

int SyncMappedFile(mapped_file_t mf) {
    if (mf.address && mf.size > 0) {
        return msync(mf.address, mf.size, MS_SYNC);
    }
    return -1;
}
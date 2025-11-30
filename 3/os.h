#pragma once

#ifdef _WIN32

#else
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef int pid_t;

typedef struct {
    void* address;
    size_t size;
} mapped_file_t;
#endif

typedef int pid_t;

pid_t CloneProcess();

int Exec(const char* path, char* argv[]);

int WaitProcess();

mapped_file_t CreateMappedFile(const char* filename, size_t size);
void* OpenMappedFile(const char* filename, size_t size);
int CloseMappedFile(mapped_file_t mf);
int SyncMappedFile(mapped_file_t mf);
#ifndef NACHOSOPENFILESTABLE_H
#define NACHOSOPENFILESTABLE_H


#include <stdio.h>

#define MAX_OPEN_FILES 32

class NachosOpenFilesTable {
public:
    NachosOpenFilesTable();
    ~NachosOpenFilesTable();

    int Open(FILE *file);
    int Close(int fileId);
    bool IsOpened(int fileId);
    FILE *GetFile(int fileId);

private:
    FILE *openFiles[MAX_OPEN_FILES];
};

#endif
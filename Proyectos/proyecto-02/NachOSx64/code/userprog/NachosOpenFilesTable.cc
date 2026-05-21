#include "NachosOpenFilesTable.h"

NachosOpenFilesTable::NachosOpenFilesTable()
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        openFiles[i] = NULL;
    }
}

NachosOpenFilesTable::~NachosOpenFilesTable()
{
    for (int i = 3; i < MAX_OPEN_FILES; i++)
    {
        if (openFiles[i] != NULL)
        {
            fclose(openFiles[i]);
            openFiles[i] = NULL;
        }
    }
}

int NachosOpenFilesTable::Open(FILE *file)
{
    for (int i = 3; i < MAX_OPEN_FILES; i++)
    {
        if (openFiles[i] == NULL)
        {
            openFiles[i] = file;
            return i;
        }
    }

    return -1;
}

int NachosOpenFilesTable::Close(int fileId)
{
    if (!IsOpened(fileId))
    {
        return -1;
    }

    fclose(openFiles[fileId]);
    openFiles[fileId] = NULL;

    return 0;
}

bool NachosOpenFilesTable::IsOpened(int fileId)
{
    return fileId >= 3 && fileId < MAX_OPEN_FILES && openFiles[fileId] != NULL;
}

FILE *NachosOpenFilesTable::GetFile(int fileId)
{
    if (!IsOpened(fileId))
    {
        return NULL;
    }

    return openFiles[fileId];
}
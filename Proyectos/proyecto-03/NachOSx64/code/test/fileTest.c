/*
    Archivo prueba para el Read, Write, Open, Close y Create
    Para la lectura de archivos más que todo
*/

#include "syscall.h"

int main()
{
    OpenFileId file;
    char buffer[6];

    Create("prueba.txt");

    file = Open("prueba.txt");
    Write("hola\n", 5, file);
    Close(file);

    file = Open("prueba.txt");
    Read(buffer, 5, file);
    Close(file);

    Write(buffer, 5, ConsoleOutput);

    Exit(0);
}
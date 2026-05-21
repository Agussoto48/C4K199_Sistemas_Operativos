#ifndef AGUA_H
#define AGUA_H
#include "synch.h"

class Agua
{
private:
    Lock *lock;
    Condition *cond;
    int hidrogenos;
    int oxigenos;
    int moleculas;

    void intentarCrearAgua();

public:
    Agua();
    ~Agua();

    void Hidrogeno(int id);
    void Oxigeno(int id);
};

#endif
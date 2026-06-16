#include "agua.h"
#include <stdio.h>

Agua::Agua()
{
    lock = new Lock("agua lock");
    cond = new Condition("agua condition");
    hidrogenos = 0;
    oxigenos = 0;
    moleculas = 0;
}

Agua::~Agua()
{
    delete lock;
    delete cond;
}

void Agua::Hidrogeno(int id)
{
    lock->Acquire();

    hidrogenos++;
    printf("Hidrogeno %d llego\n", id);

    intentarCrearAgua();

    lock->Release();
}

void Agua::Oxigeno(int id)
{
    lock->Acquire();

    oxigenos++;
    printf("Oxigeno %d llego\n", id);

    intentarCrearAgua();

    lock->Release();
}

void Agua::intentarCrearAgua()
{
    while (hidrogenos >= 2 && oxigenos >= 1)
    {
        hidrogenos -= 2;
        oxigenos -= 1;
        moleculas++;

        printf("Molecula de agua #%d creada\n", moleculas);
    }
}
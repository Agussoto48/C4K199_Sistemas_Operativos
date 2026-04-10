#pragma once
/**
 *  C++ class to encapsulate Unix message passing intrinsic structures and system calls
 *
 *  Author: Programacion Concurrente (Francisco Arroyo)
 *  Version: 2026/Mar
 *
 **/

#include <unistd.h> // pid_t definition
#include <sys/msg.h>
#include <stdexcept>
#include <cstring>

#define ROJO   "\033[31m"
#define VERDE  "\033[32m"
#define AZUL   "\033[34m"
#define AMARILLO "\033[33m"
#define RESET  "\033[0m"

struct Mensaje_carro
{
    long mtype;
};

struct Mensaje_evento
{
    long mtype;
    int carros_fila;
    int carros_totales;
    int idFila;
};


class Buzon
{
private:
    int id;
    pid_t owner;
    bool creador;

public:
    Buzon(int key, bool crear = true);
    ~Buzon();

    ssize_t Enviar(const void *buffer, size_t size, long type = 1);
    ssize_t Recibir(void *buffer, size_t size, long type = 1);
    int getId() { return id;}
};

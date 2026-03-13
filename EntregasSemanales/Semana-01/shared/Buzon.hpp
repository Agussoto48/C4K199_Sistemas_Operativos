#pragma once
/**
  *  C++ class to encapsulate Unix message passing intrinsic structures and system calls
  *
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *  Version: 2026/Mar
  *
 **/

#include <unistd.h>	// pid_t definition
#include<sys/msg.h>
#include<stdexcept>
#include<cstring>

#define KEY 0xC4E199
class Buzon {
private:
    int id;		
    pid_t owner;	
    bool creador;
public:
    Buzon(bool crear = true);
    ~Buzon();

    ssize_t Enviar( const void *buffer, size_t size, long type = 1);
    ssize_t Recibir(void *buffer, size_t size, long type = 1);
};

#include "Buzon.hpp"
#include<iostream>

class Carro
{
private:
    int key;
    int id;
    int calleId;
    Buzon buzon;
public:
    Carro(int key, int id , int calleId)
    : key(key), id(id) , calleId(calleId), buzon(key, false)
    {

    }
    ~Carro() = default;

    void entrar()
    {
        std::cout << "Carro " << id << " entra a calle " << calleId << std::endl;
    }
    void esperar()
    {
        Mensaje_carro msg;
        msg.mtype = 1;
        buzon.Recibir((void *) &msg, sizeof(msg));
    }
    void salir()
    {
        std::cout << "Carro " << id << " entra a rotonda de calle " << calleId << ", termina" << std::endl;
    }
};
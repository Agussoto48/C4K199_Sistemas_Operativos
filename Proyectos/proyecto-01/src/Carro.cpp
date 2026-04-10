#include "Carro.hpp"

Carro::Carro(int key, int id, int calleId)
    : key(key), id(id), calleId(calleId), buzon(key, false)
{
}
Carro::~Carro() = default;
void Carro::entrar()
{
    std::cout << AMARILLO << "Carro " << id << " entra a calle " << calleId << RESET << std::endl;
}
void Carro::esperar()
{
    Mensaje_carro msg;
    msg.mtype = 1;
    buzon.Recibir((void *)&msg, sizeof(msg));
}
void Carro::salir()
{
    std::cout << VERDE << "Carro " << id << " entra a rotonda de calle " << calleId << ", termina" << RESET << std::endl;
}
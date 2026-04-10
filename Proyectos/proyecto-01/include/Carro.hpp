#include "Buzon.hpp"

class Carro
{
private:
    int key;
    int calleId;
    Buzon buzon;
public:
    Carro(int key)
    : key(key), buzon(key, false)
    {

    }
    ~Carro();

    void entrar();
    void esperar();
    void salir();
};
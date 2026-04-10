#include "Buzon.hpp"
#include <iostream>

class Carro
{
private:
    int key;
    int id;
    int calleId;
    Buzon buzon;

public:
    /**
     * @brief Constructor de Carro
     *
     * @param key Llave del buzón que usará el carro.
     * @param id Identificador del carro.
     * @param calleId Calle o fila a la que pertenece el carro.
     */
    Carro(int key, int id, int calleId);
    // Destructor
    ~Carro();
    /**
     * @brief Simula la llegada del carro a una calle.
     */
    void entrar();
    /**
     * @brief Espera autorización para avanzar.
     */
    void esperar();
    /**
     * @brief Simula la salida de la calle hacia la rotonda.
     */
    void salir();
};
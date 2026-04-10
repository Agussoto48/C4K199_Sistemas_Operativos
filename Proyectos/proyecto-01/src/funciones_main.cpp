#include "funciones_main.hpp"

void vaciar(int key, int &numCarros, int &carrosTotales, Buzon *buzon, bool mismaFila)
{
    Mensaje_carro msg;
    msg.mtype = 1;

    for (int i = (mismaFila) ? 1 : 0; i < numCarros; i++)
    {
        buzon->Enviar((void *)&msg, sizeof(msg));
    }
    if (!mismaFila)
    {
        carrosTotales -= numCarros;
        numCarros = 0;
    }
    else
    {
        carrosTotales -= numCarros - 1;
        numCarros = 1;
    }
}
int fila_mayor(std::vector<int> filas, int nFilas)
{
    int mayor = 0;
    for (int i = 0; i < nFilas; i++)
    {
        std::cout << "Fila " << i + 1 << ": " << filas[i] << std::endl;
        if (mayor < filas[i])
            mayor = filas[i];
    }
    int i = 0;
    while (mayor != filas[i])
        i++;
    return i;
}

void vaciarTodo(std::vector<int> filas, std::vector<Buzon *> carros_buzones, int nFilas)
{
    Mensaje_carro msg;
    msg.mtype = 1;
    for (int i = 0; i < nFilas; i++)
    {
        for (int j = 0; j < filas[i]; j++)
        {
            carros_buzones[i]->Enviar((void *)&msg, sizeof(msg));
        }
    }
}
bool sameLine(int fila_a_vaciar, int fila_asignada)
{
    return fila_a_vaciar == fila_asignada;
}

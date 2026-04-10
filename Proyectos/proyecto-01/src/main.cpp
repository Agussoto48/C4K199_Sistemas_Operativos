#include "Carro.hpp"
#include <vector>
#include <sys/wait.h>

#define nCarros 50
#define nFilas 3
#define maxCarros 10

#define KEY_EVENTO 0xC4E199

int main()
{
    std::vector<int> filas(nFilas);
    std::vector<Carro*> carros(nCarros);
    int totalCarros = 0;
    int decidir_fila = 0;
    Buzon sePaso(KEY_EVENTO, true);
    Mensaje_evento msg_ev;
    msg_ev.mtype = 1;

    for (int i = 0; i < nCarros; i++)
    {   
        filas[i % nFilas] += 1;
        carros[i] = new Carro(i % nFilas);
        if (!fork())
        {
            if(totalCarros > maxCarros)
            {
                
                sePaso.Enviar((void*) &msg_ev, sizeof(msg_ev));
            }
            exit(0);
        }
        else if(totalCarros > maxCarros)
        {
            sePaso.Recibir((void*) &msg_ev, sizeof(msg_ev));
            filas[i % nFilas] = msg_ev.carros_fila;
            totalCarros = msg_ev.carros_totales;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        wait(NULL);
    }
    return 0;
}
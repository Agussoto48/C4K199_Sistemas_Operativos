#include "Carro.hpp"
#include <vector>
#include <sys/wait.h>

#define nCarros 50
#define nFilas 3
#define maxCarros 10

#define KEY_EVENTO 0xC4E199
#define KEY_BASE 0x5000

void vaciar(int key, int &numCarros, int &carrosTotales, Buzon *buzon);
int fila_mayor(std::vector<int> filas);

int main()
{
    std::vector<int> filas(nFilas);
    std::vector<Carro*> carros(nCarros);
    std::vector<Buzon*> carros_buzones(nFilas);

    for(int i = 0; i < nFilas; i++)
    {
        carros_buzones[i] = new Buzon(KEY_BASE + i);
    }


    int totalCarros = 0;
    Buzon sePaso(KEY_EVENTO, true);
    Mensaje_evento msg_ev;
    msg_ev.mtype = 1;

    int key_fila = 0;

    for (int i = 0; i < nCarros; i++)
    {   
        key_fila = KEY_BASE + (i % nFilas); 
        filas[i % nFilas] += 1;
        totalCarros += 1;
        carros[i] = new Carro(key_fila, i+1 , i % nFilas + 1);
        if (!fork())
        {
            carros[i]->entrar();
            if(totalCarros > maxCarros)
            {   
                std::cout << "SE PASO EL LIMITE DE " << maxCarros << std::endl;
                int fila_a_vaciar = fila_mayor(filas);
                key_fila = KEY_BASE + fila_a_vaciar;
                vaciar(key_fila, filas[fila_a_vaciar], totalCarros, carros_buzones[fila_a_vaciar]);
                msg_ev.carros_fila = filas[fila_a_vaciar];
                msg_ev.carros_totales = totalCarros;
                msg_ev.idFila = fila_a_vaciar;
                sePaso.Enviar((void*) &msg_ev, sizeof(msg_ev));
            }
            carros[i]->esperar();
            carros[i]->salir();
            exit(0);
        }
        else if(totalCarros > maxCarros)
        {
            sePaso.Recibir((void*) &msg_ev, sizeof(msg_ev));
            int idFila = msg_ev.idFila;
            filas[idFila] = msg_ev.carros_fila;
            totalCarros = msg_ev.carros_totales;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        wait(NULL);
    }

    for(int i = 0; i < nFilas; i++)
    {
        delete carros_buzones[i];
    }
    for(int i = 0; i < nCarros; i++)
    {
        delete carros[i];
    }
    return 0;
}

void vaciar(int key, int &numCarros, int &carrosTotales, Buzon *buzon)
{
    Mensaje_carro msg;
    msg.mtype = 1;
    for(int i = 0; i < numCarros; i++)
    {
        buzon->Enviar((void*) &msg, sizeof(msg));
    }
    carrosTotales -= numCarros;
    numCarros = 0;
}
int fila_mayor(std::vector<int> filas)
{   
    int mayor = 0;
    for(int i = 0; i < nFilas; i++)
    {
        std::cout << "Fila " << i+1 << ": " << filas[i] << std::endl;
        if(mayor < filas[i])
            mayor = filas[i];
    }
    int i = 0;    
    while(mayor != filas[i])
        i++;
    
    return i;

}
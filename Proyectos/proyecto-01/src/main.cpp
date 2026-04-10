#include "Carro.hpp"
#include "funciones_main.hpp"
#include <vector>
#include <sys/wait.h>

#define KEY_EVENTO 0xC4E199
#define KEY_BASE 0x5000

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Uso: " << argv[0] << " <nCarros> <nFilas> <maxCarros>" << std::endl;
        return 1;
    }
    int nCarros = atoi(argv[1]);
    int nFilas = atoi(argv[2]);
    int maxCarros = atoi(argv[3]);

    std::vector<int> filas(nFilas);
    std::vector<Carro *> carros(nCarros);
    std::vector<Buzon *> carros_buzones(nFilas);

    //Crea los buzones de cada calle/fila con su propia clave
    for (int i = 0; i < nFilas; i++)
    {
        carros_buzones[i] = new Buzon(KEY_BASE + i);
    }

    int totalCarros = 0;
    //Buzon para comunicar con padre
    Buzon sePaso(KEY_EVENTO, true);
    Mensaje_evento msg_ev;
    msg_ev.mtype = 1;

    int key_fila = 0;

    for (int i = 0; i < nCarros; i++)
    {
        key_fila = KEY_BASE + (i % nFilas);
        filas[i % nFilas] += 1;
        totalCarros += 1;
        carros[i] = new Carro(key_fila, i + 1, i % nFilas + 1);
        if (!fork())
        {
            carros[i]->entrar();
            //Se paso del maxCarros
            if (totalCarros > maxCarros)
            {
                std::cout << ROJO << "SE PASO EL LIMITE DE " << RESET << maxCarros << std::endl;
                std::cout << AZUL << "Total carros antes de vaciar: " << totalCarros << RESET << std::endl;
                int fila_a_vaciar = fila_mayor(filas, nFilas);
                key_fila = KEY_BASE + fila_a_vaciar;
                vaciar(key_fila, filas[fila_a_vaciar], totalCarros, carros_buzones[fila_a_vaciar], sameLine(fila_a_vaciar, i % nFilas));
                msg_ev.carros_fila = filas[fila_a_vaciar];
                msg_ev.carros_totales = totalCarros;
                msg_ev.idFila = fila_a_vaciar;
                sePaso.Enviar((void *)&msg_ev, sizeof(msg_ev));
            }
            //Ultimo carro
            if (i == nCarros - 1)
            {
                usleep(500000);
                std::cout << AMARILLO << "Ya no quedan mas carros, se da el paso a los que quedan" << RESET << std::endl;
                vaciarTodo(filas, carros_buzones, nFilas);
            }
            carros[i]->esperar();
            carros[i]->salir();
            exit(0);
        }
        //Actualizar datos para el padre para que el siguiente hijo tenga la informacion correcta
        else if (totalCarros > maxCarros)
        {
            sePaso.Recibir((void *)&msg_ev, sizeof(msg_ev));
            int idFila = msg_ev.idFila;
            filas[idFila] = msg_ev.carros_fila;
            totalCarros = msg_ev.carros_totales;
            for (int i = 0; i < nFilas; i++)
            {
                std::cout << AZUL << "Fila " << i + 1 << ": " << filas[i] << RESET << std::endl;
            }
            std::cout << AZUL << "Total carros despues de vaciar: " << totalCarros << RESET << std::endl;
        }
    }

    for (int i = 0; i < nCarros; i++)
    {
        wait(NULL);
    }

    for (int i = 0; i < nFilas; i++)
    {
        delete carros_buzones[i];
    }
    for (int i = 0; i < nCarros; i++)
    {
        delete carros[i];
    }
    return 0;
}
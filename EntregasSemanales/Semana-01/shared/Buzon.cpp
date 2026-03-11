#include "Buzon.hpp"

Buzon::Buzon(bool crear)
{
    if (crear)
    {
        id = msgget(KEY, 0600 | IPC_CREAT);
        creador = true;
    }
    else
    {
        id = msgget(KEY, 0600);
        creador = false;
    }
    if (id == -1)
    {
        if (errno == ENOENT)
        {
            perror("No se ha creado un buzón aún, nada que recibir: ");
        }
        else
        {
            throw std::runtime_error("Error al crear el buzon");
        }
    }
    owner = getpid();
}
Buzon::~Buzon()
{
    // El que recibe el mensaje se encarga de cerrar el buzon, ya que si se cierra antes de recibirlo no llega nada
    if (!creador)
    {
        msgctl(id, IPC_RMID, NULL);
    }
}
ssize_t Buzon::Enviar(const void *buffer, size_t size, long type)
{
    return msgsnd(this->id, buffer, size, IPC_NOWAIT);
}
ssize_t Buzon::Recibir(void *buffer, size_t size, long type)
{
    return msgrcv(id, buffer, size, type, IPC_NOWAIT);
}

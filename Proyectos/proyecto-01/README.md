# Rotonda

Este proyecto simula el comportamiento de una rotonda a la que llegan carros desde varias calles, cada carro es representado por un proceso, y la comunicación entre ellos se maneja mediante buzones y paso de mensajes.

La idea principal es controlar qué fila de carros puede avanzar hacia la rotonda según la cantidad total de carros esperando, si el total de carros en espera supera un límite máximo, se permite el paso a la fila con más carros acumulados, pero si varias filas tienen la misma cantidad máxima, se escoge la de menor índice.

## Qué hace el programa

Cada carro sigue este flujo general:

- Llega a una calle y se registra en su fila.
- Aumenta la cantidad total de carros en espera.
- Si no se supera el límite máximo, espera su turno.
- Si se supera el límite, se vacía la fila con más carros.
- Cuando recibe permiso, entra a la rotonda y termina.

## Parámetros de entrada

El programa recibe 3 argumentos por línea de comandos:

```bash
./bin/rotonda <nCarros> <nFilas> <maxCarros>
```

## Makefile

### Compilación
- `make` o `make build` : Compila el programa y genera el ejecutable en `bin/rotonda`.

### Ejecución
- `make run ARGS="10 2 3"` : Ejecuta el programa con argumentos por línea de comandos.  
  En este caso:
  - `10` = cantidad de carros
  - `2` = cantidad de filas
  - `3` = máximo de carros permitidos antes de vaciar una fila

### Limpieza
- `make clean` : Elimina el ejecutable generado dentro de la carpeta `bin`.

## Autor

- Agustín Soto | C4K199
# Proyecto 3 - Memoria Virtual en NachOS

## Descripción

En este proyecto se implementó el manejo de memoria virtual para NachOS, incorporando Demand Paging, reemplazo de páginas mediante LRU, administración de un archivo SWAP y soporte para TLB.

El objetivo fue permitir la ejecución de programas cuando el espacio disponible en memoria física es insuficiente, cargando únicamente las páginas necesarias y reemplazándolas cuando la memoria se encuentra llena.

---

# Funcionalidades implementadas

## Llamados al sistema

Se verificó el funcionamiento de los llamados al sistema utilizados durante las pruebas del proyecto, incluyendo la correcta ejecución de `Exit`, `Exec`, `Join` y `Fork`, necesarios para ejecutar los programas de prueba y el programa `todos`.

---

## Cambios realizados en AddrSpace

Se modificó la administración del espacio de direcciones para soportar memoria virtual.

Entre los principales cambios se encuentran:

- Inicialización de las páginas como inválidas.
- Implementación de Demand Paging.
- Carga de páginas únicamente cuando ocurre un Page Fault.
- Administración de los marcos de memoria física.
- Selección de páginas víctimas mediante LRU.
- Actualización de la información de las páginas durante los reemplazos.
- Implementación de `SaveState()` y `RestoreState()` para sincronizar correctamente la información cuando se utiliza TLB.

---

## Manejo de excepciones

El manejador de excepciones fue actualizado para soportar Page Faults.

Durante una excepción se realiza:

- Obtención de la dirección que produjo el fallo.
- Determinación de la página virtual correspondiente.
- Verificación de si la página ya se encuentra en memoria.
- Carga de la página cuando es necesario.
- Actualización de la TLB cuando está habilitada.

---

## Demand Paging

Las páginas ya no se cargan completamente al crear un proceso.

Cada página es cargada únicamente cuando el programa intenta acceder a ella por primera vez.

---

## Algoritmo LRU

El reemplazo de páginas utiliza el algoritmo **Least Recently Used (LRU)**.

Cada página mantiene un registro del último instante en que fue utilizada, permitiendo seleccionar la página menos utilizada recientemente cuando la memoria física se encuentra llena.

Este mismo criterio también se utiliza para el reemplazo de entradas dentro de la TLB.

---

## Manejo de SWAP

Se implementó un archivo SWAP para almacenar temporalmente páginas que deben salir de memoria física.

Las funcionalidades implementadas incluyen:

- Creación del archivo SWAP.
- Escritura de páginas modificadas.
- Recuperación de páginas previamente almacenadas.
- Administración de la ubicación de cada página dentro del archivo.
- Actualización de la tabla de páginas durante las operaciones de SWAP.

---

## Manejo de la TLB

Se agregó soporte para trabajar utilizando la TLB cuando `USE_TLB` está habilitado.

La implementación incluye:

- Actualización automática de la TLB cuando ocurre un TLB Miss.
- Sincronización de los bits `use`, `dirty` y `lastUsed` entre la TLB y la tabla de páginas.
- Limpieza de la TLB durante los cambios de contexto mediante `SaveState()` y `RestoreState()`.
- Reemplazo de entradas utilizando LRU.
- Actualización de la información de la página víctima y de la página entrante.

---

# Casos de prueba

Se realizaron pruebas utilizando los programas proporcionados por NachOS.

Programas ejecutados:

- halt
- sort
- matmult
- copy
- shell
- todos

Las pruebas fueron realizadas con:

- 32 páginas físicas.
- 4 páginas físicas (modificando `NumPhysPages` en `machine.h`).

Se verificó el funcionamiento del reemplazo de páginas, el uso del archivo SWAP y el manejo de memoria virtual bajo ambas configuraciones.

---

# Compilación

Desde la carpeta `code`:

```bash
make clean
make
```

O bien desde:

```bash
cd userprog
make clean
make
```

---

# Ejecución

La ejecución se hace desde **/userprog**

Ejemplos:

```bash
./nachos -x ../test/halt
./nachos -x ../test/sort
./nachos -x ../test/matmult
./nachos -x ../test/copy
./nachos -x ../test/todos
./nachos -x ../test/shell
```

Para las pruebas con cuatro páginas físicas:

1. Modificar `NumPhysPages` en `machine.h`.
2. Compilar nuevamente el proyecto.
3. Ejecutar los programas de prueba.

---

# Archivos modificados

Las principales modificaciones del proyecto se realizaron en:

- `userprog/addrspace.cc`
- `userprog/addrspace.h`
- `userprog/exception.cc`
- `machine/machine.h` (para las pruebas con distinta cantidad de páginas físicas)

---

# Estudiante

- Agustín Soto
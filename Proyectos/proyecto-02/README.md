# Proyecto #2 - NachOS

## Descripción general

Este proyecto consistió en ampliar la funcionalidad de NachOS mediante la implementación y mejora de diferentes mecanismos relacionados con la administración de memoria, ejecución de procesos, manejo de archivos y comunicación mediante sockets.

El trabajo se realizó siguiendo el enunciado del proyecto y las guías proporcionadas, tomando como base mi versión de NachOS utilizada previamente en el Proyecto Integrador.

---

# Objetivos implementados

Durante el desarrollo se trabajó en los siguientes componentes:

* Administración de memoria física mediante BitMap.
* Manejo de archivos:

  * Create
  * Open
  * Read
  * Write
  * Close
* Manejo de procesos:

  * Exit
  * Yield
  * Fork
  * Exec
  * Join
* Implementación básica de sockets:

  * Socket
  * Connect
* Ejecución de programas de prueba:

  * shell
  * copy
  * todos

---

# Archivos modificados

Los principales cambios se realizaron en los siguientes archivos:

## userprog/system.cc

* Inicialización del BitMap global.
* Preparación de estructuras necesarias para la administración de memoria.

## userprog/addrspace.h

* Incorporación de estructuras auxiliares para manejo de memoria.
* Constructor de copia para soporte de Fork.
* Declaración de estructuras relacionadas con páginas propias.

## userprog/addrspace.cc

* Asignación dinámica de páginas físicas utilizando BitMap.
* Liberación de páginas en el destructor.
* Implementación del constructor de copia utilizado por Fork.
* Carga de segmentos del ejecutable.

## userprog/exception.cc

Implementación de los principales system calls:

* Create
* Open
* Read
* Write
* Close
* Exit
* Yield
* Fork
* Exec
* Join
* Socket
* Connect

Además se incorporaron funciones auxiliares para:

* Lectura de strings desde memoria de usuario.
* Registro de procesos.
* Creación de hilos para Fork.
* Ejecución de programas mediante Exec.
* Avance del Program Counter.

## userprog/NachosOpenFilesTable.h
## userprog/NachosOpenFilesTable.cc

* Implementación de una tabla de archivos abiertos.
* Asociación de identificadores de archivo con FILE*.
* Soporte para Open y Close.

## machine/machine.h

* Ajustes relacionados con la cantidad de páginas físicas disponibles durante las pruebas.
* Utilizado para validar escenarios de ejecución que requerían una mayor cantidad de memoria.

---

# Resumen del desarrollo

## 1. Actualización del BitMap

Se reemplazó la asignación secuencial de páginas por una administración basada en BitMap.

Cada vez que un proceso necesita memoria:

* Se solicita una página libre al BitMap.
* Se registra su uso.
* Al finalizar el proceso, la página es liberada nuevamente.

Esto permitió reutilizar correctamente la memoria física disponible.

---

## 2. Manejo de archivos

Se implementó una tabla de archivos abiertos para permitir:

* Abrir archivos.
* Leer archivos.
* Escribir archivos.
* Cerrar archivos.

Los identificadores utilizados por los programas de usuario son traducidos internamente a estructuras FILE*.

---

## 3. Manejo de procesos

### Exit

Permite finalizar un proceso y liberar su memoria.

### Yield

Permite ceder voluntariamente el procesador a otro hilo.

### Fork

Crea un nuevo hilo utilizando una copia del espacio de direcciones del proceso actual.

### Exec

Carga un programa ejecutable y crea un nuevo hilo encargado de ejecutarlo.

### Join

Permite esperar la finalización de un proceso previamente creado mediante Exec.

---

## 4. Sockets

Se implementó soporte básico para:

### Socket

Creación de sockets TCP.

### Connect

Conexión hacia un servidor remoto utilizando dirección IP y puerto, estas funcionalidades permiten ejecutar el programa de prueba socket.

---

# Pruebas realizadas (Todas desde userprog)

## Shell

```bash
./nachos -x ../test/shell
```

Permite ejecutar programas de usuario desde una interfaz interactiva.

Como se hace desde userprog hay que poner las direcciones correctas, por ejemplo, una vez ejecutandose shell, si se quiere ejecutar memory, el input correcto sería **../test/memory**.

---

## Copy

```bash
cp nachos nachos.1
./nachos -x ../test/copy
```

Genera el archivo:

```text
nachos.2
```

como copia de:

```text
nachos.1
```

---

## Todos

```bash
cp nachos nachos.1
./nachos -x ../test/todos
```

Verifica el funcionamiento conjunto de:

* Exec
* Join
* Fork
* Open
* Read
* Write
* Close

## Ejecución de todos desde shell

Además de ejecutar el programa `todos` directamente, se verificó su funcionamiento desde el programa de usuario `shell`.

Prueba realizada:

```bash
./nachos -x ../test/shell
```
y dentro de shell
```bash
../test/todos
```
Si se quiere terminar de ejecutar shell, sería:
```bash
../test/halt
```




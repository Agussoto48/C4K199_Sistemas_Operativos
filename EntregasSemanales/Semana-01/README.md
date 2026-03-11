# Buzones

## Descripción

Semana introductoria para los buzones

El `Makefile` permite compilar y ejecutar fácilmente los distintos programas, organizándolos en la carpeta `bin`.

## Makefile

- `make` o `make default` : Compila todos los programas (con clases y sin clases).  
- `make buildClase` : Compila los programas de **enviar y recibir con clases**.  
- `make buildSinClase` : Compila los programas de **enviar y recibir sin clases**.  

### Ejecución
- `make runEC` : Ejecuta el programa **enviar con clases**.  
- `make runRC` : Ejecuta el programa **recibir con clases**.  
- `make runE` : Ejecuta el programa **enviar sin clases**.  
- `make runR` : Ejecuta el programa **recibir sin clases**.  

### Limpieza
- `make clean` : Elimina todos los ejecutables dentro de la carpeta `bin`.

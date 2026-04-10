#include "Buzon.hpp"
#include<vector>
#include<iostream>


void vaciar(int key, int &numCarros, int &carrosTotales, Buzon *buzon, bool mismaFila);
void vaciarTodo(std::vector<int> filas, std::vector<Buzon*> carros_buzones, int nFilas);
bool sameLine(int fila_a_vaciar, int fila_asignada);
int fila_mayor(std::vector<int> filas, int nFilas);

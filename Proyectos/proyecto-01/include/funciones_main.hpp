#include "Buzon.hpp"
#include <vector>
#include <iostream>

/**
 * @brief Vacía la fila con mayor cantidad de carros.
 *
 * Envía los mensajes necesarios para permitir el paso de los carros de la fila
 * seleccionada, si el carro que acaba de entrar pertenece a esa misma fila,
 * no se vacía por completo y se deja ese carro en espera.
 *
 * @param key Llave del buzón de la fila que se va a vaciar.
 * @param numCarros Cantidad de carros en la fila a vaciar.
 * @param carrosTotales Cantidad total de carros en todas las filas.
 * @param buzon Buzón asociado a la fila que se va a vaciar.
 * @param mismaFila Indica si el carro que acaba de entrar pertenece a la misma fila.
 */
void vaciar(int key, int &numCarros, int &carrosTotales, Buzon *buzon, bool mismaFila);

/**
 * @brief Vacía todas las filas de la simulación.
 *
 * @param filas Vector con la cantidad de carros en cada fila.
 * @param carros_buzones Vector con los buzones asociados a cada fila.
 * @param nFilas Cantidad total de filas.
 */
void vaciarTodo(std::vector<int> filas, std::vector<Buzon *> carros_buzones, int nFilas);

/**
 * @brief Verifica si dos filas son la misma.
 *
 * @param fila_a_vaciar Índice de la fila que se desea vaciar.
 * @param fila_asignada Índice de la fila asignada al carro actual.
 * @return true si ambas filas son iguales.
 * @return false si son diferentes.
 */
bool sameLine(int fila_a_vaciar, int fila_asignada);

/**
 * @brief Obtiene la fila con mayor cantidad de carros.
 *
 * Si dos o más filas tienen la misma cantidad máxima de carros,
 * se selecciona la que tenga el índice más bajo.
 *
 * @param filas Vector con la cantidad de carros en cada fila.
 * @param nFilas Cantidad total de filas.
 * @return int Índice de la fila con más carros.
 */
int fila_mayor(std::vector<int> filas, int nFilas);

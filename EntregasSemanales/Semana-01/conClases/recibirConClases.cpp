/**
  *  C++ program to receive messages via operating system message passing queues
  *
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *
  *  Version: 2026/March
  *
 **/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<iostream>
#include "../shared/Buzon.hpp"
#include "../shared/estructuras.hpp"
#define LABEL_SIZE 64


int main( int argc, char ** argv ) {
   struct msg A;
   int st;
   Buzon m(false);

   st = m.Recibir((void *) &A, sizeof(A), 2026); 
   while ( st > 0 ) {
      printf("Label: %s, times %d \n", A.mtext, A.times);
      st = m.Recibir( (void *)  &A, sizeof(A) ,2026 );
   }
}


/**
  *  C++ program to receive messages via operating system message passing queues
  *
  *  No C++ class implementation
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
#include "../shared/estructuras.hpp"

#define KEY 0xA12345
#define LABEL_SIZE 64


int main( int argc, char ** argv ) {

   struct msg A;
   int id, size, st;

   id = msgget( KEY, 0600 );
   if ( -1 == id ) {
      perror("t0-recibe: ");
      exit(1);
   }

   size = sizeof( A );

   st = msgrcv( id,  &A, size, 2026, IPC_NOWAIT );
   while ( st > 0 ) {
      printf("Label: %s, times %d \n", A.mtext, A.times );
      st = msgrcv( id,  &A, size, 2026, IPC_NOWAIT );
   }

   msgctl( id, IPC_RMID, NULL );

}
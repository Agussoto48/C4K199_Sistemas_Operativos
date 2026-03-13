
/**
  *  C++ program to send messages via operating system message passing queues
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

const char * html_labels[] = {
   "a",
   "b",
   "c",
   "d",
   "e",
   "li",
   ""
};

int main( int argc, char ** argv ) {

   struct msg A;
   int id, i, size, st;

   id = msgget( KEY, 0600 | IPC_CREAT );
   if ( -1 == id ) {
      perror("t0-envia: ");
      exit(1);
   }

   A.mtype = 2026;
   i = 0;

   while ( strlen(html_labels[ i ] ) ) {
      A.times = i;
      strcpy( A.mtext, html_labels[ i ] );
      size = sizeof( A );
      st = msgsnd( id,  &A, size, IPC_NOWAIT );
      printf("Label: %s, status %d \n", html_labels[ i ], st );
      i++;
   }

   return 0;
}
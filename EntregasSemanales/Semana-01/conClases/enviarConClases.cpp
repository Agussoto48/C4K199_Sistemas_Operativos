

/**
  *  C++ program to send messages via operating system message passing queues
  *
  *  Author: Programacion Concurrente (Francisco Arroyo)
  *
  *  Version: 2026/March
  *
 **/


#define LABEL_SIZE 64
#include <stdio.h>
#include <string.h>
#include "../shared/Buzon.hpp"
#include "../shared/estructuras.hpp"

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


   int i;
   ssize_t st;
   struct msg A;
   Buzon m(true);
   A.mtype = 2026;
   i = 0;

   while ( strlen(html_labels[ i ] ) ) {
      strcpy(A.mtext, html_labels[ i ]);
      A.times = i;
      st = m.Enviar((void *) &A, sizeof(A), 2026);  // Send a message with 2026 type, (label,n)
      printf("Label: %s, status %ld \n", html_labels[ i ], st );
      i++;
   }

}

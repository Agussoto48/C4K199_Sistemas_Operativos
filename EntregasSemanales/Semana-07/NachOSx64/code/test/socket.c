#include "syscall.h"


char * strcat(char *s, const char *append) {
   char *save = s;
   for (; *s; ++s);
   while ((*s++ = *append++) != '\0');
   return(save);
}

int strlen(const char *str) {
   const char *s;

   for (s = str; *s; ++s)
                ;
   return (s - str);
}


int main() {
   int id;
   char a[ 512 ];
   char * req = "GET /aArt/index.php?disk=Disk-01&fig=";
   char s[ 50 ];

   id = Socket( AF_INET_NachOS, SOCK_STREAM_NachOS );
   Connect( id, "163.178.104.62", 80 );
   Write( "Figura: ", 8, 1 );
   Read( s, 50, 0 );
   req = strcat( req, s );
   req = strcat( req, " HTTP/1.0\r\nUser-Agent: nachos\r\n\r\n\r\n" );
   Write( req, strlen( req ), 1 );
//   Write( "GET / HTTP/1.0\r\n\r\n", 32, id );
   Write( req, strlen( req ), id );
   Read( a, 512, id );
   Write( a, 512, 1 );
   Close( id );

}


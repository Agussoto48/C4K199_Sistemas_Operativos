#include "syscall.h"

int globalCounter = 0;

void sum() {

	int i;

	for ( i=0; i<100; i++) {
	    globalCounter++;
	}
	Exit(i);
}

int main()
{
	int i=0;
	
	globalCounter++;

	Fork(sum);
	Yield();


	globalCounter++;
	Fork(sum);

	Yield();
	
	globalCounter++;
	Fork(sum);

	Yield();
	
	globalCounter++;
	Exit( globalCounter ); 
}


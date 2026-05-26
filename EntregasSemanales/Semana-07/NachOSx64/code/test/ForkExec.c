#include "syscall.h"

void usememory(){
	Exec("../test/memory");
}

int main() {
	int i=0;

	
		Fork(usememory);
		Yield();
	
	Exit(0);
}


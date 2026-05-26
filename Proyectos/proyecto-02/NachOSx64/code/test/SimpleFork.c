#include "syscall.h"


void sum() {
	Write("hijo\n", 5, ConsoleOutput);
	Exit(10);

}

int main() {
	Write("padre inicio\n", 13, ConsoleOutput);
	Fork(sum);
	Yield();

	Fork(sum);
	Yield();
	
	Write("padre fin\n", 10, ConsoleOutput);
	Exit(6); 
}


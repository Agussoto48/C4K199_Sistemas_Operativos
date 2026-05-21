#include "syscall.h"


void sum() {

	Exit(10);

}

int main() {
	Fork(sum);
	Yield();

	Fork(sum);
	Yield();

	Exit(6); 
}


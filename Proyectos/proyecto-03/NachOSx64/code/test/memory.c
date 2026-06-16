#include "syscall.h"

int array[128];

int main() {
	int i;
	for (i = 0; i < 128; i++) 
		array[i] = 2025;

	Exit(0);
}


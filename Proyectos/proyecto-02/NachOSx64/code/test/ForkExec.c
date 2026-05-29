#include "syscall.h"

void usememory(){
	Exec("../test/memory");
}

int main() {

    Write("antes\n", 6, ConsoleOutput);
    Exec("../test/memory");
	Yield();
    Write("despues\n", 8, ConsoleOutput);
    Exit(0);

}


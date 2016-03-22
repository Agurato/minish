#include <stdio.h>
#include <stdlib.h>
#include "commands.h"

int main(int argc, char **argv) {
	redirectOutput("input1\n", "fichierTest.txt", 1);
	redirectOutput("input2\n", "fichierTest.txt", 1);
	redirectOutput("input3\n", "fichierTest.txt", 2);

	return 0;
}

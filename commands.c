#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* Handles redirection of output
*
* @param input : string to push in file
* @param filename : file name
* @param mode :
*		= 1 : simple redirection (erase file)
* 		= 2 : extended redirection (append to file)
*
* @return result of execution
*/
int redirectOutput(char *input, char *filename, int mode) {

	if (mode == 1) {
		FILE *file = fopen(filename, "w");

		fprintf(file, "%s", input);

		fclose(file);
	}
	else if(mode == 2) {
		FILE *file = fopen(filename, "a");

		fprintf(file, "%s", input);

		fclose(file);
	}
	else {
		return 1;
	}
	return 0;
}

char* handleDollar(char* string) {
	char* command = malloc(sizeof(string)+20);

	//FILE *output;

	strcat(command, "echo ");
	strcat(command, string);
	strcat(command, " > tempDollar.tmp");




	return string;
}

#include "lib.h"

int main(int argc, char **argv){
	char *buffer;
	char *separator = " ";
	char *token;
	char **charac;

	int background = 0;

	pid_t pidF;

	do {
		int i=0;

		buffer = calloc(1024, sizeof(char));

		charac = calloc(1024, sizeof(char*));

		printf("> ");
		fgets(buffer, 1024*sizeof(char), stdin);
		buffer[strlen(buffer)-1] = '\0';

		token = strtok(buffer, separator);

		while(token != NULL) {
			if(token[0] != '&') {
				background = 0;
				charac[i] = token;
			}
			else {
				background = 1;
			}

			i++;
			token = strtok(NULL, separator);
		}

		switch(pidF = fork()){
			case -1:
				perror("Fork failed");
				return 1;
			case 0:
				if(strcmp(buffer, "exit") != 0) {

					if(execvp(charac[0], charac) == -1){
						perror("Commande non valide");
						return 2;
					}

				}
				else {
					return 0;
				}
		}

		if(!background) {
			while(waitpid(pidF,0,0) < 0);
		}

	} while(strcmp(buffer, "exit") != 0);

	return 0;
}

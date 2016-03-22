#include "lib.h"

int main(int argc, char **argv){
	char *buffer;
	char *separator = " ";
	char *token;
	char **charac;

	pid_t pidF;

	do {
		int i=0;

		buffer = calloc(1024, sizeof(char));

		charac = calloc(1024, sizeof(char*));

		printf("> ");
		fgets(buffer, 1024*sizeof(char), stdin);
		buffer[strlen(buffer)-1] = '\0';

		//printf("%s\n", buffer);

		token = strtok(buffer, separator);

		while(token != NULL){
			charac[i] = token;

			i++;
			token = strtok(NULL, separator);
		}
		//charac[i-1][strlen(charac[i-1])-1] = '\0';

		switch(pidF = fork()){
			case -1:
				perror("Fork failed");
				return 1;
			case 0:
				if(strcmp(buffer, "exit") != 0) {
					printf("Fils : %s", charac[0]);
					if(execvp(charac[0], charac) == -1){
						perror("Commande non valide");
						return 2;
					}
				}
				else {
					return 0;
				}
		}

		while(waitpid(pidF,0,0) < 0);

	} while(strcmp(buffer, "exit") != 0);

	return 0;
}

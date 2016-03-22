#include "lib.h"

int main(int argc, char **argv){
	char *buffer;
	char *separator = " ";

	pid_t pidF;

	do {
		int i=0;
		char *token;
		char **charac;

		buffer = calloc(1024, sizeof(char));

		charac = calloc(1024, sizeof(char*));

		printf("> ");
		fgets(buffer, 1024, stdin);
		buffer[sizeof(buffer)-1] = '\0';

		//printf("%s\n", buffer);

		token = strtok(buffer, separator);

		while(token != NULL){
			//*(charac+i) = calloc(sizeof(char), 1+strlen(token));
			//strcpy(*(charac+i), token);

			charac[i] = token;

			printf("%d - %s : %ld\n", i, token, strlen(token));

			//printf("%s", *(charac+i));
			i++;
			/*
			token = calloc(1024, sizeof(char));
			*/
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

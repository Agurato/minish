#include "lib.h"

int readkey();
int main(int argc, char **argv){
	char *buffer;
	char *separator = " ";

	pid_t pidF;

	do {
		char *token ;
		char **charac;
		char *save;


		buffer = calloc(1024, sizeof(char));

		charac = calloc(1024, sizeof(char*));

		printf("> ");
		scanf("%s", buffer);
		buffer[sizeof(buffer)-2] = '\0';


		token = strtok_r(buffer,separator,&save);

		int i=0;

		while(token != NULL){
			*(charac+i) = token;
			i++;
			token = strtok_r(NULL, separator,&save);
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

		while(waitpid(pidF,0,0) < 0);

	} while(strcmp(buffer, "exit") != 0);

	return 0;
}

int readkey(){
	struct termios init;
	struct termios config;
	int key;
	tcgetattr(STDIN_FILENO, &init);
	config = init;
	config.c_lflag &= ~ICANON;
	config.c_lflag &= ~ECHO;

	tcsetattr(STDIN_FILENO, TCSANOW, &config);
	key = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &init);
	return key;
}

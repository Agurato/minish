#include "lib.h"

int readkey();
int main(int argc, char **argv){
	char buffer[1024];
	char *separator = { " " };
	pid_t pidF;
	int key = 1;
	int i=0;
	while(key != '\0'){
		char *token ;
		char **charac;
		char *save;
		read(STDIN_FILENO, buffer,1024);
		token = strtok_r(buffer,separator,&save);

		while(token != NULL){
			printf("TOKEN = %s\n ", token);
			*(charac+i) = token;
			//token = '\0';
			i++;
			token = strtok_r(NULL, separator,&save);
		}

		switch(pidF = fork()){
			case -1:
				perror("Fork failed");
				return 1;
			case 0:
				if(execvp(charac[0], charac) == -1){
					perror("Commande non valide");
					return 2;
				}
		}
		while(waitpid(pidF,0,0) < 0);
	}



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

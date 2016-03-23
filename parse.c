/*
 *	Programme qui simule un bash
 *	1 Thread pour prendre le texte
 *	1 qui copie la commande et qui créé des processus 
 *	exitProgramme = 1 => Arrêt de tous les thread
 *
 * */
#include "lib.h"
#include <pthread.h>

#define MAXWORD 100
#define MAXCHAR 60

pthread_t parsing;
pthread_t treatment;

pthread_mutex_t mutex;
pthread_cond_t endCmd;
pthread_cond_t endExecuting;


char **commande;

int exitProgram = 0;
int endExecute = 1;

static int c = 0;
static int nombreMot = 0;


/*
 * Thread qui parse ce que l'on saisi
 * TODO :
 *	Retour en arriere pris en compte
 *	Prise en compte de & => dire a execute DONE
 */
void *parse(void *t){
	int k;

	struct termios initial;
	struct termios config;

	tcgetattr(STDIN_FILENO, &initial);
	config = initial;
	config.c_lflag &= ~ICANON;
	config.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &config);

	pthread_mutex_lock(&mutex);
	commande = malloc(MAXWORD * sizeof(char*));
	for(k=0;k<MAXWORD;k++){
		commande[k] = calloc(MAXCHAR , sizeof(char));
	}
	pthread_mutex_unlock(&mutex);

	while(c != 27){
		int j = 0;

		pthread_mutex_lock(&mutex);
		if(!endExecute){
			pthread_cond_wait(&endExecuting, &mutex);
		}
		pthread_mutex_unlock(&mutex);

		fputs("$ ", stdout);
		endExecute = 0;
		do{
			c = getchar();
			pthread_mutex_lock(&mutex);
			if(c > 32 && c != 127){
				printf("%c",(char)c);
				commande[nombreMot][j++] = (char)c;
			}
			if(c == 32){
				printf(" ");
				if(nombreMot < MAXWORD){
					j = 0;
					nombreMot++;
				}else{
					c = 10;
					puts("Trop de mots, FIN");
				}

			}
			if(c == 127){
				if(j > 0){
					j--;
					commande[nombreMot][j] = '\0';
				}
				else if(j==0 && nombreMot>0){
					nombreMot--;
					j = sizeof(commande[nombreMot])-1;
				}
			}
			pthread_mutex_unlock(&mutex);
		}while(c != 27 && c != 10);
		puts(" ");
		nombreMot++;
		pthread_cond_signal(&endCmd);
	}
	exitProgram = 1;
	puts(" ");
	tcsetattr(STDIN_FILENO, TCSANOW, &initial);
	return t;
}

/*
 * Thread qui execture les commandes qu'on demande
 * TODO
 *	Prise en comtpe de & DONE, voir si ca peut pas creer des problemes
 */
void *execute(void *p){
	char **cmd;
	int k;
	int pid;
	int background = 0;

	while(!exitProgram){
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&endCmd, &mutex);

		if(commande[0][0] != '\0'){
			if(!strncmp(commande[nombreMot-1], "&", sizeof(commande[nombreMot-1]))){
				//	printf("commande = %s\n",commande[nombreMot-1]);
				nombreMot--;
				background = 1;
				commande[nombreMot][0] = '\0';
			}
			cmd = calloc(nombreMot+1 , sizeof(char*));
			for(k=0;k<nombreMot;k++){
				cmd[k] = calloc(sizeof(commande[k]), sizeof(char));
				strcpy(cmd[k], commande[k]);
			}
			endExecute = 0;
			switch(pid = fork()){
				case -1:
					exitProgram = 1;
					return p;
				case 0:
					//printf("cmd[0] = %s\n",commande[0]);
					if(execvp(*cmd,cmd) == -1){
						fprintf(stderr,"Erreur lors de execvp\n");
						return p;
					}
			}
			if(!background){
				while(waitpid(pid,0,0) <= 0);
			}
		}

		pthread_cond_signal(&endExecuting);
		endExecute = 1;
		background = 0;

		for(k=0;k<MAXWORD;k++){
			commande[k] = calloc(MAXCHAR, sizeof(char));
		}
		nombreMot = 0;
		pthread_mutex_unlock(&mutex);
	}
	return p;
}

int main(int argc, char **argv){

	pthread_create(&parsing, 0, parse, 0);
	pthread_create(&treatment, 0, execute, 0);
	if(pthread_mutex_init(&mutex,0) == -1){
		fprintf(stderr,"Failed to create mutex\n");
		return 1;
	}
	if(pthread_cond_init(&endCmd, 0) == -1){
		fprintf(stderr,"Condition can't be initialized\n");
		pthread_mutex_destroy(&mutex);
		return 2;
	}
	if(pthread_cond_init(&endExecuting, 0) == -1){
		fprintf(stderr,"Condition can't be initialized\n");
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&endCmd);
	}


	pthread_join(parsing, 0);
	pthread_join(treatment,0);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&endCmd);
	pthread_cond_destroy(&endExecuting);

	puts("Thank you for your use");
	return 0;
}

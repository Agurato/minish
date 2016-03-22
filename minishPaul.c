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


char **commande;

int exitProgram = 0;

static int c = 0;

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
		int i = 0;
		int j = 0;

		fputs("$ ", stdout);
		do{
			c = getchar();
			if(c > 32 && c != 127){
				printf("%c",(char)c);
				//commande[i] = calloc(strlen(commande[i])+1, sizeof(char));
				//printf("%ld",sizeof(commande[i]));
				pthread_mutex_lock(&mutex);
				commande[i][j++] = (char)c;
				pthread_mutex_unlock(&mutex);
			}
			if(c == 32){
				printf(" ");
				if(i < MAXWORD){
					j = 0;
					i++;
				}else{
					c = 10;
					puts("Trop de mots, FIN");
				}

			}
		}while(c != 27 && c != 10);
			puts(" ");
			i++;
		//	for(j=0;j<i;j++){
		//		printf("%s ", commande[j]);
				pthread_cond_signal(&endCmd);
		//	}
			puts(" ");
	}
	exitProgram = 1;
	puts(" ");
	tcsetattr(STDIN_FILENO, TCSANOW, &initial);
	return t;
}

void *execute(void *p){
	int k,l;
	while(!exitProgram){
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&endCmd, &mutex);
		switch(fork()){
			case -1:
				exitProgram = 1;
				return p;
			case 0:
				printf("commande[0] = %s\n",commande[0]);
				if(execvp(commande[0],commande) == -1){
					fprintf(stderr,"Erreur lors de execvp\n");
					return p;
				}
		}
		for(k=0;k<MAXWORD;k++){
			for(l=0;l<MAXCHAR;l++)
			commande[k][l] = '\0';
		}
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

	pthread_join(parsing, 0);
	pthread_join(treatment,0);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&endCmd);

	puts("Thank you for your use");
	return 0;
}

/*
 *	Programme sh
 *	1 Thread pour prendre le texte
 *	1 qui copie la commande et qui créé des processus 
 *	exitProgramme = 1 => Arrêt de tous les threads
 *	TODO
 *	  history avec sauvegarde
 *	  Remove \n of history live
 *	  pipe
 *	  redirection
 *	  define les différentes touches (pour rendre le code POSIX)
 *	  free when we leave, create function cleanup
 *	  
 *	  getcwd pour connaitre sa position absolue
 * */
#include "lib.h"
#include <pthread.h>

#define MAXWORD 100
#define MAXCHAR 200
#define MAXSAVE 50

typedef struct{
	char ***string;
	int *taille;
}Save;

pthread_t parsing;
pthread_t treatment;

pthread_mutex_t mutex;
pthread_cond_t endCmd;
pthread_cond_t endExecuting;


char **commande;
Save save;

int exitProgram = 0;
int endExecute = 1;

int currentSave = 0;
int numberSave = 0;

/*
 * Static inutile
 */
static int c = 0;
static int nombreMot = 0;
static int nombreLettre = 0;


/*
 * Fonction that parse char when user type it
 * This function doesn't parse when user confirm his command
 */
int parse(){
	if(c == 27){
		struct termios init;
		struct termios config;
		int d;
		tcgetattr(STDIN_FILENO, &init);
		config = init;
		config.c_cc[VMIN] = 0;
		config.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &config);
		if((d = getchar()) == 91){
			if((d=getchar()) == 65){
				int k = 0;
			//	for(k=0;k<save.taille[numberSave-1];k++)
			//		printf("%s",save.string[numberSave-1][k]);
				if(currentSave >= 0){
					nombreMot = save.taille[currentSave];
					commande = calloc(MAXWORD,sizeof(char*));
				//	printf("currentSave = %d ",currentSave);
					for(k=0;k<MAXWORD;k++){
						commande[k] = calloc(MAXCHAR, sizeof(char));
					}
					//	if(save[currentSave][k] != NULL)
					for(k=0;k<save.taille[currentSave];k++){
					//	printf("%s", save.string[currentSave][k]);
						strcpy(commande[k], save.string[currentSave][k]);
					}
					/*\0332K = Erase line \033u = cursor go to the last save*/
					printf("\033[2K \033[u \033[1D\033[1D\033[1D\033[1D$ ");
					for(k=0;k<nombreMot;k++){
						printf("%s ", commande[k]);
					}
					currentSave--;
					nombreLettre = 0;
				//	nombreMot++;
				}
			}
			c = 0;
		}
		tcsetattr(STDIN_FILENO, TCSANOW, &init);
	}
	else if(c > 32 && c != 127){//Displayable 
		if(nombreLettre < MAXCHAR){
			printf("%c",(char)c);
			commande[nombreMot][nombreLettre++] = (char)c;
		}else{
			fprintf(stdin,"Taille dépasse la limite autorisée\n");
			c = 10;
		}
	}
	else if(c == 32){ //Space
		printf(" ");
		if(nombreMot < MAXWORD){
			nombreLettre = 0;
			nombreMot++;
		}else{
			c = 10;
			puts("Trop de mots, FIN");
		}

	}
	else if(c == 127){ //BackSpace
		if(nombreLettre > 0){
			nombreLettre--;
			commande[nombreMot][nombreLettre] = '\0';
			printf("\033[1D \033[1D");
		}
		else if(nombreLettre==0 && nombreMot>0){
			nombreMot--;
			nombreLettre = sizeof(commande[nombreMot])-1;
			printf("\033[1D \033[1D");
		}
	}
	return 0;
}


/*
 * Thread qui parse ce que l'on saisi
 * TODO :
 *	Retour en arriere pris en compte
 *	Prise en compte de & => dire a execute DONE
 */
void *readInput(void *t){
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
		pthread_mutex_lock(&mutex);
		if(!endExecute){
			pthread_cond_wait(&endExecuting, &mutex);
		}
		pthread_mutex_unlock(&mutex);

		fputs("$ ", stdout);
		printf("\033[s");
		endExecute = 0;
		do{
			c = getchar();

			pthread_mutex_lock(&mutex);
			parse();
			pthread_mutex_unlock(&mutex);

		}while(c != 27 && c != 10);
		puts(" ");
		nombreMot++;
		
		/*
		 * We save the commande that we type
		 */
		if(numberSave < MAXSAVE){
			int i;
			save.string[numberSave] = calloc(nombreMot+1, sizeof(char*));
			for(i = 0;i < nombreMot;i++){
				save.string[numberSave][i] = calloc(strlen(commande[i]),sizeof(char));
				strcpy(save.string[numberSave][i], commande[i]);
			}
			save.taille[numberSave] = nombreMot;
			numberSave++;
		}
		pthread_cond_signal(&endCmd);

		nombreLettre = 0;
		currentSave = numberSave-1;
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
			if(strlen(commande[nombreMot-1]) == 1){
				if(!strncmp(commande[nombreMot-1], "&", 1)){
					nombreMot--;
					background = 1;
					commande[nombreMot][0] = '\0';
				}
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
					if(background)
						printf("pid : %d\n", getpid());
					if(execvp(*cmd,cmd) == -1){
						//	fprintf(stderr,"Erreur lors de execvp\n");
						perror("Commande non trouvée ");
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

/**
 * Cree les différents threads et attend leur fin
 */
int main(int argc, char **argv){
	save.string = calloc(MAXSAVE, sizeof(char**));
	save.taille = malloc(MAXSAVE * sizeof(int));

	pthread_create(&parsing, 0, readInput, 0);
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

	/*
	for(i=0;i<numberSave;i++){
		for(j = 0;j < save.taille[j];j++)
			free(save.string[i][j]);
	}
	free(save.taille);
	*/
	puts("\t\t**************************************");
	puts("\t\t\tThank you for your use");
	puts("\t\t**************************************");

	return 0;
}

/*void cleanup(){}*/

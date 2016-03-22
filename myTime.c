#include "lib.h"

int main(int argc, char **argv){
    struct timeval start, end;
    double tempsEnd, tempsStart;
    double uEnd, uStart;
    int pid;

    if(argc < 2){
        perror("Il faut un paramètre à cette fonction");
        return -2;
    }
    gettimeofday(&start , NULL);
    switch(pid = fork()){
        case -1:
            perror("Erreur lors de la création du processus\n");
            return -1;
        case 0:
           if(execvp(argv[1], argv+1) == -1){
               perror("Erreur commande invalide \n");
               return -3;
           }
    }
    while(waitpid(0,0,0) < 0);
    gettimeofday(&end, NULL);
    uEnd = end.tv_usec;
    tempsEnd = end.tv_sec + (uEnd/1000000);
    uStart = start.tv_usec;
    tempsStart = start.tv_sec + (uStart/1000000);

    printf("Le temps d'execution est : %lf\n", (tempsEnd - tempsStart));
    return 0;
}

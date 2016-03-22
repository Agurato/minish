#include "lib.h"

int main(int argc, char **argv){
	
	while(key != '\0'){
		key = readkey();
		switch(pidF = fork()){
			case -1:
				perror("Fork failed");
				return 1;
			case 0:



	return 0;
}

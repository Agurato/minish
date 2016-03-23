all :
	gcc -o minish minish.c -Wall -pedantic

minishPaul : parse.c lib.h
	gcc -o paulMinish parse.c -pthread
	make print

print : paulMinish
	echo "Pour mettre ce bash en standard : sudo cp ./<Exe> /usr/bin/"
	echo "puis chsh /usr/bin/<Exe>"
	echo "pour revenir a l'ancien : chsh /bin/bash"

clean : minish
	rm minish paulMinish

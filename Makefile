all :
	gcc -o minish minish.c -Wall -pedantic
clean : minish
	rm minish

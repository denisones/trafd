# If you have trouble => ers@cool.net.ru

INCLUDE=/usr/local/include/pgsql/
LIBS=/usr/local/lib

all:
	clear
	gcc -I$(INCLUDE) -L$(LIBS) -o trafd trafd.c -lpq
	gcc -o accidental accidental.c	
clean: 
	rm -f trafd

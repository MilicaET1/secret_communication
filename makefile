.PHONY : all
all : server klijent
server: server.c
	gcc -o server server.c
klijent: client.c
	gcc -o klijent client.c 
	

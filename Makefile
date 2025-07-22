
all: kort.o player.o
	gcc $^ 

ai: kort.o computer_player.o
	gcc $^ 

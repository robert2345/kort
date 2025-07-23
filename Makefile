
all: kort.o player.o player_common.o
	gcc $^ 

ai: kort.o computer_player.o player_common.o
	gcc $^ 

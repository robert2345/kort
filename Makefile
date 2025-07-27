
all: kort.o player.o player_common.o
	gcc $^ -o kort

ai: kort.o computer_player.o player_common.o
	gcc $^ -o kort

clean:
	rm *.o
	rm *.exe

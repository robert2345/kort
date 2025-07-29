all: human ai test

human:  kort.o player.o player_common.o
	gcc $^ -o kort

ai: kort.o computer_player.o player_common.o
	gcc -g $^ -o ai_kort

test:
	$(MAKE) -C ./test

clean:
	rm *.o
	rm *.exe
	$(MAKE) -C ./test clean


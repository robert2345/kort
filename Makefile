all: human ai tests

human:  kort.o player.o player_common.o
	gcc $^ -o kort

ai: kort.o computer_player.o player_common.o
	gcc $^ -o ai_kort

tests:
	$(MAKE) test

clean:
	rm *.o
	rm *.exe


#include "../player_common.c"
#include "../computer_player.c"


static int test_play_ace()
{

	return 0;
}

int main(int argc, char **argv)
{
	int res = 0;
	res |= test_play_ace();

	if (res)
		fprintf(stderr, "Tests failed!\n");
	return res;
}

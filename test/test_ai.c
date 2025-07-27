#include "../player_common.h"
#include "../player.h"
#include <stdio.h>

#define CHECK(x) if (x) {\
			return 0; \
		} else { \
			fprintf(stderr, "%s, line %d, failed. (%s)\n", __func__, __LINE__, #x); \
			fprintf(stderr, "Action: %d, from_index %d, count %d\n", pa.action, pa.from_index, pa.count); \
			return -1; \
		}

struct state default_state = {
	.hand = {(struct card){0,0}, (struct card){1,1}, (struct card){2,2}, no_card},
	.top_of_aces = {no_card,no_card,no_card,no_card},

	.top_of_kings = {no_card,no_card,no_card,no_card},
	.top_of_piles = {{0 ,0},no_card,no_card,no_card,no_card,no_card,no_card,no_card,no_card,no_card,no_card,no_card,no_card},
	.current_pile = 0,
	
};

static int test_play_ace()
{
	struct state state = default_state;
	struct player_action pa = {};

	state.hand[0] = (struct card){.value = 0, .color = 0};
	player_prompt_action(&state, &pa);
	CHECK (pa.action == ACTION_PLAY_FROM_HAND_TO_ACES && pa.from_index == 0)
}

int main(int argc, char **argv)
{
	int res = 0;
	res |= test_play_ace();

	if (res)
		fprintf(stderr, "Tests failed!\n");
	return res;
}

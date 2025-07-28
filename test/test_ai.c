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
	.hand = {(struct card){0,0}, no_card},
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

// Something in a pile takes precedence over something on hand
static int test_play_ace_pile()
{
	struct state state = default_state;
	struct player_action pa = {};

	state.top_of_piles[1] = (struct card){.value = 0, .color = 1};
	state.hand[0] = (struct card){.value = 0, .color = 0};
	player_prompt_action(&state, &pa);
	CHECK (pa.action == ACTION_PLAY_FROM_PILE_TO_ACES && pa.from_index == 1)
}

// if we can play between aces and kings, set it up for playing max number of pile-tops
static int test_move_to_play_series()
{
	struct state state = default_state;
	struct player_action pa = {};

	state.top_of_piles[0] = (struct card){.value = 6, .color = 1};
	state.top_of_piles[1] = (struct card){.value = 8, .color = 1};
	state.top_of_piles[2] = (struct card){.value = 9, .color = 1};
	state.top_of_piles[3] = (struct card){.value = 10, .color = 1};
	state.top_of_aces[1] = (struct card){.value = 6, .color = 1};
	state.top_of_kings[1] = (struct card){.value = 7, .color = 1};
	player_prompt_action(&state, &pa);
	// We expect the 7 to be moved to the 6 on aces, so that 8,9 and 10 can be played on top
	CHECK (pa.action == ACTION_PLAY_FROM_KINGS_TO_ACES && pa.from_index == 1 && pa.count == 1);
}

// Imagine if you have two cards that you can play from hand, j
static int test_think_one_step_ahead_1()
{
	struct state state = default_state;
	struct player_action pa = {};
	state.top_of_piles[1] = (struct card){.value = 8, .color = 2}; // would be nice to play this to king
								       //
	state.top_of_kings[2] = (struct card){.value = 10, .color = 2};// but we need one inbetween
	state.top_of_piles[2] = (struct card){.value = 9, .color = 2}; // we also have this guy that can be put there
								       //
	state.top_of_aces[2] = (struct card){.value = 8, .color = 2}; // but we may accidently play it on ace instead

	player_prompt_action(&state, &pa);
	CHECK (pa.action == ACTION_PLAY_FROM_PILE_TO_KINGS && pa.from_index == 2);

}

static int test_think_one_step_ahead_2()
{
	struct state state = default_state;
	struct player_action pa = {};
	state.top_of_piles[1] = (struct card){.value = 8, .color = 2}; // would be nice to play this to ace
								       //
	state.top_of_aces[2] = (struct card){.value = 6, .color = 2};// but we need one inbetween
	state.top_of_piles[2] = (struct card){.value = 7, .color = 2}; // we also have this guy that can be put there
								       //
	state.top_of_kings[2] = (struct card){.value = 8, .color = 2}; // but we may accidently play it on king instead

	player_prompt_action(&state, &pa);
	CHECK (pa.action == ACTION_PLAY_FROM_PILE_TO_ACES && pa.from_index == 2);

}

int main(int argc, char **argv)
{
	int res = 0;
	res |= test_play_ace();
	res |= test_play_ace_pile();
	res |= test_move_to_play_series();
	res |= test_think_one_step_ahead_1();
	res |= test_think_one_step_ahead_2();

	if (res)
		fprintf(stderr, "Tests failed!\n");
	return res;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "kort.h"
#include "player_common.h"
#include "player.h"

static struct state last_state;
static enum action last_action;

// Does not check the order of the hand, just its's contents and the top of
// other piles. I.e. no cards have been played!
bool state_is_equal(struct state s0, struct state s1)
{

	// check all piles except hand
	for (int i = 0; i < NBR_VALUES; i ++)
	{
		if (i == s0.current_pile)
			continue;
		if (!cards_are_equal(s0.top_of_piles[i], s1.top_of_piles[i])) {
			//printf("Piles are not equal\n");
			return false;
		}
	}
	// check hand
	if (calc_hand_size(s0) != calc_hand_size(s1))
		return false;
	
	printf("States are equal\n");
	return true;
}

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	//printf("\e[1;1H\e[2J");
	print_state(state);
	usleep(1000000);

	// if state is changed -> play
	if (!state_is_equal(last_state, state)) {
		last_action = pa->action = ACTION_CUSTOM_2;
		//printf("State has changed, so play\n");;
	}
	else if (last_action == ACTION_CUSTOM_2 ){
		last_action = pa->action = ACTION_CUSTOM_1;

	}
	else {
		last_action = pa->action = ACTION_PUT_HAND_DOWN;
	}

	oldest_state = last_state;
	last_state = state;


	if (pa->action == ACTION_CUSTOM_2)
	{
		//Custom automatic play action. Player defined.
		//printf("Trying to play\n");
		if (try_play_pile(state, pa)) {
			number_of_reorders_since_last_state_change = 0;
			consequent_reorders=0;
			return;
		}
		if (try_play_hand(state, pa)) {
			number_of_reorders_since_last_state_change = 0;
			consequent_reorders=0;
			return;
		}
		//printf("Failed to play");
		// fall back to reorder
		last_action = pa->action = ACTION_CUSTOM_1;
	}

	if (pa->action == ACTION_CUSTOM_1)
	{
		//Custom automatic reorder action. Player defined.

		//printf("Trying to reorder\n");
		pa->action = ACTION_REORDER_HAND;
		calc_new_hand_order(state, pa->new_hand_order);
		number_of_reorders_since_last_state_change++;
		consequent_reorders++;
		return;

	}


}

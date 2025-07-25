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
	if (s0.current_pile != s1.current_pile)
		return false;
	if (calc_hand_size(s0) != calc_hand_size(s1))
		return false;
	
	printf("States are equal\n");
	return true;
}

enum src {
	SOURCE_NONE,
	SOURCE_PILE,
	SOURCE_HAND,
};

struct dream_candidate {
	enum src source;
	char value;
};


static int compare_dream_candidates(const void *p, const void *q) {
    struct dream_candidate dc0 = *(const struct dream_candidate *)p;
    struct dream_candidate dc1 = *(const struct dream_candidate *)q;

    if (dc0.value > dc1.value)
	    return 1;
    if (dc0.value < dc1.value)
	    return -1;
    return 0;
}

static bool find_dream_card(struct state state, int color)
{
	int nbr_candidates = 0;
	// check all top of piles except current_pile to see if there is a card of this color
	// if there are several, choose.
	struct dream_candidate candidates[2*NBR_VALUES];
	int hand_size = calc_hand_size(state);
	for (int i = 0; i < NBR_VALUES; i++)
	{
		if (i == state.current_pile)
			continue;
		if (state.top_of_piles[i].color == color) {
			candidates[nbr_candidates].value = state.top_of_piles[i].value;
			candidates[nbr_candidates].source = SOURCE_PILE;
			nbr_candidates++;
		}
	}
	for (int i = 0; i < hand_size; i++)
	{
		if (state.hand->cards[i]->color == color) {
			candidates[nbr_candidates].value = state.hand->cards[i]->value;
			candidates[nbr_candidates].source = SOURCE_PILE;
			nbr_candidates++;
		}

	}
	qsort(candidates,nbr_candidates, sizeof(candidates), compare_dream_candidates);

	{
		int last_value = -2;
		int streak_start = 0;
		int current_hand_cards = 0;
		int best_hand_cards = 1000;
		int best_index = 0;
		int best_streak = -1;
		for (int i = 0; i < (nbr_candidates-1); i++)
		{
			if (candidates[i].value != (last_value +1)) 
			{
				//check if new best
				int streak = streak_start - i +1;
				if (streak > best_streak || (streak == best_streak && best_hand_cards > current_hand_cards)){
					best_index = streak_start;
					best_hand_cards = current_hand_cards;
					best_streak = streak;
				}

				streak_start = i;
				current_hand_cards = 0;
			}
			last_value = candidates[i].value;
			if (candidates[i].source == SOURCE_HAND)
				current_hand_cards++;

		}


	}

}

static bool move_kings_and_aces(struct state state, struct player_action *pa)
{
	for (int i = 0; i < 4; i++)
	{
		if (state.top_of_kings[i].value == (1+state.top_of_aces[i].value)) {
			char s[100];
			do {
				printf("WARNING! it is possible to move cards between aces and kings piles! Press 'o' to proceed.\n");
				gets(s);
			} while (s[0] != 'o');

			// find the best card in top of pile
			if (find_dream_card(state, i)) {

				return true;
			}
		}
	}
	return false;
}

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	//printf("\e[1;1H\e[2J");
	print_state(state);
	usleep(100000);

	getchar();

	// if cards can be moved between kings/aces, we should make use of this to play one or several cards from the piles.
	if (last_action != ACTION_PLAY_FROM_ACES_TO_KINGS &&
	    last_action != ACTION_PLAY_FROM_KINGS_TO_ACES &&
	    move_kings_and_aces)
		last_action = pa->action;

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

	last_state = state;


	if (pa->action == ACTION_CUSTOM_2)
	{
		//Custom automatic play action. Player defined.
		//printf("Trying to play\n");
		if (try_play_pile(state, pa)) {
			return;
		}
		if (try_play_hand(state, pa)) {
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
		return;

	}


}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "kort.h"
#include "player_common.h"
#include "player.h"

static struct state last_state;
static enum action last_action;
static struct card piles[NBR_VALUES][MAX_CARDS];

// Does not check the order of the hand, just its's contents and the top of
// other piles. I.e. no cards have been played!
bool state_is_equal(struct state *s0, struct state *s1)
{

	// check all piles except hand
	for (int i = 0; i < NBR_VALUES; i ++)
	{
		if (i == s0->current_pile)
			continue;
		if (!cards_are_equal(s0->top_of_piles[i], s1->top_of_piles[i])) {
			return false;
		}
	}

	// check hand
	if (s0->current_pile != s1->current_pile) {
		return false;
	}
	if (calc_hand_size(s0) != calc_hand_size(s1)) {
		return false;
	}
	
	return true;
}

static int find_dream_card(struct state *state, int color)
{
	int nbr_candidates = 0;
	// check all top of piles except current_pile to see if there is a card of this color
	// if there are several, choose.
	struct dream_candidate candidates[2*NBR_VALUES];
	int hand_size = calc_hand_size(state);
	for (int i = 0; i < NBR_VALUES; i++)
	{
		if (i == state->current_pile)
			continue;
		if (state->top_of_piles[i].color == color) {
			candidates[nbr_candidates].value = state->top_of_piles[i].value;
			candidates[nbr_candidates].source = SOURCE_PILE;
			nbr_candidates++;
		}
	}
	for (int i = 0; i < hand_size; i++)
	{
		if (state->hand[i].color == color) {
			candidates[nbr_candidates].value = state->hand[i].value;
			candidates[nbr_candidates].source = SOURCE_HAND;
			nbr_candidates++;
		}

	}

	if (nbr_candidates){
		int last_value = -2;
		int streak_start = 0;
		int streak;
		int current_hand_cards = 0;
		int best_hand_cards = 1000;
		int best_index = 0;
		int best_streak = -1;
		qsort(candidates,nbr_candidates, sizeof(*candidates), compare_dream_candidates);
		for (int i = 0; i < (nbr_candidates); i++)
		{
			if (i == (nbr_candidates-1) || candidates[i].value != (last_value +1)) 
			{
				//check if new best
				if (streak > best_streak || (streak == best_streak && best_hand_cards > current_hand_cards)){
					best_index = streak_start;
					best_hand_cards = current_hand_cards;
					best_streak = streak;
				}

				current_hand_cards = 0;
				streak_start = i;
				streak = 1;
			}
			else if (candidates[i].value == (last_value)) 
			{
				// convoluted way to say that if the card is both on pile and hand it should be considered on pile
				if ((candidates[i].source != SOURCE_HAND && candidates[i-1].source == SOURCE_HAND)
				||(candidates[i].source == SOURCE_HAND && candidates[i-1].source != SOURCE_HAND))
					current_hand_cards--;

				// also don't consider the streak longer because the same card appears twice.
			}
			else {
				streak++;
			}

			last_value = candidates[i].value;
			if (candidates[i].source == SOURCE_HAND)
				current_hand_cards++;

		}

		if (best_index >= 0) {
			return candidates[best_index].value;
		}
		else
			return -1;


	}
	return -1;
}

static bool move_kings_and_aces(struct state *state, struct player_action *pa)
{
	for (int i = 0; i < 4; i++)
	{
		if (state->top_of_kings[i].value == (1+state->top_of_aces[i].value)) {
			char dream_value = 0; 
			char s[100];
			// find the best card in top of pile
			dream_value = find_dream_card(state, i);
			if (dream_value != -1) {
				int cards_to_move = state->top_of_aces[i].value - (dream_value -1); // Aim is to make room for the dream cards, so that the ace card should be one lower.

				if (cards_to_move > 0)
					pa->action = ACTION_PLAY_FROM_ACES_TO_KINGS;
				else if (cards_to_move < 0)
					pa->action = ACTION_PLAY_FROM_KINGS_TO_ACES;
				else
					continue;
				pa->count = abs(cards_to_move);
				pa->from_index = i;
				return true;
			}
		}
	}
	return false;
}

static void store_hand(struct state *state)
{
	int i = 0;
	int hand_size = calc_hand_size(state);
	for (; i < hand_size; i++)
	{
		piles[state->current_pile][i] = state->hand[i];

	}
	piles[state->current_pile][i] = no_card;
}

static void validate_piles(struct state *state)
{
	for (int i = 0; i < NBR_VALUES; i++) 
		if (!cards_are_equal(no_card,piles[i][0]) && cards_are_equal(state->top_of_piles[i],piles[i][0]))
			fprintf(stderr, "PILES ARE NOT IN SYNC WITH GAME!!!!");
}

void player_prompt_action(struct state *state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	//printf("\e[1;1H\e[2J");
	//print_state(state);
	//usleep(100000);

	// if cards can be moved between kings/aces, we should make use of this to play one or several cards from the piles.
	if (last_action != ACTION_PLAY_FROM_ACES_TO_KINGS &&
	    last_action != ACTION_PLAY_FROM_KINGS_TO_ACES &&
	    move_kings_and_aces(state, pa)) {
		last_action = pa->action;
		last_state = *state;
		return;
	}

	if (last_action == ACTION_PLAY_FROM_ACES_TO_KINGS ||
	    last_action == ACTION_PLAY_FROM_KINGS_TO_ACES) {
		last_action = pa->action = ACTION_CUSTOM_2;
	}
	else if (!state_is_equal(&last_state, state)) {
		last_action = pa->action = ACTION_CUSTOM_2;
	}
	else if (last_action == ACTION_CUSTOM_2 ){
		last_action = pa->action = ACTION_CUSTOM_1;

	}
	else {
		store_hand(state);
		last_action = pa->action = ACTION_PUT_HAND_DOWN;
	}

	last_state = *state;


	if (pa->action == ACTION_CUSTOM_2)
	{
		struct card (*piles_copy)[MAX_CARDS] = malloc(sizeof(piles));
		memcpy(piles_copy, piles, sizeof(piles));
		//Custom automatic play action. Player defined.
		if (try_play(state, piles_copy, pa)) {
			free(piles_copy);
			return;
		}
		// fall back to reorder
		last_action = pa->action = ACTION_CUSTOM_1;
		free(piles_copy);
	}

	if (pa->action == ACTION_CUSTOM_1)
	{
		//Custom automatic reorder action. Player defined.
		struct card (*piles_copy)[MAX_CARDS] = malloc(sizeof(piles));
		memcpy(piles_copy, piles, sizeof(piles));
		pa->action = ACTION_REORDER_HAND;
		calc_new_hand_order(state, piles_copy, pa->new_hand_order);
		free(piles_copy);
		return;

	}


}

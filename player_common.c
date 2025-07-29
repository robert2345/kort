
#include <stdio.h>
#include <stdlib.h>

#include "player_common.h"
#include "kort.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"

static char *symbols[4] = {
	"♣",
	"\x1B[31m♦\x1B[0m",
	"\x1B[31m♥\x1B[0m",
	"♠"
};

static char *values[NBR_VALUES] = {
	"A ",
	"2 ",
	"3 ",
	"4 ",
	"5 ",
	"6 ",
	"7 ",
	"8 ",
	"9 ",
	"10",
	"J ",
	"Q ",
	"K ",
};

struct card_weight {
	int index;
	int weight;
};


int calc_hand_size(const struct state *state)
{
	struct card *card_p;
	int i = 0;
	while (!cards_are_equal(no_card,state->hand[i])) {i++;};
	return i;
}

void print_aces(struct state *state)
{
	printf("\nAces: ");
	for (int i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state->top_of_aces[i], no_card))
			printf(" %s%s ", symbols[i], values[state->top_of_aces[i].value]);
		else 
			printf(" %s- ", symbols[i]);
	}
}

void print_kings(struct state *state)
{
	printf("Kings: ");
	for (int i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state->top_of_kings[i], no_card))
			printf(" %s%s ", symbols[i], values[state->top_of_kings[i].value]);
		else 
			printf(" %s- ", symbols[i]);
	}
}

void print_piles(struct state *state)
{

	int i;
	printf("Piles:\n");
	for (i = 0; i < NBR_VALUES; i ++)
	{
		printf("%02d      ", i); 

	}
	printf("\n");
	for (i = 0; i < NBR_VALUES; i ++)
	{
		if (i == state->current_pile)
			printf("hand    "); 
		else
			printf("%s%s     ", symbols[state->top_of_piles[i].color], values[state->top_of_piles[i].value]); 

	}

}

void print_hand_not_reordered(struct state *state, int *order, int order_count)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);
	int i = 0;
	while ((card_p = &state->hand[i]) && !cards_are_equal(*card_p,no_card))
	{
		bool no_print = false;
		if (order) {
			for (int j = 0 ; j < order_count; j++)
			{
				if (order[j] == i)
					no_print = true;
			}
		}

		if (!no_print) printf("%d: %s%s     ", i, symbols[card_p->color], values[card_p->value]); 
		i++;
	}

	printf("\n");

}

void print_hand(struct state *state)
{
	printf("\nHand:\n");
	print_hand_not_reordered(state, NULL, 0);
}

void print_state(struct state *state)
{

	print_aces(state);
	print_kings(state);
	printf("\n");
	print_piles(state);
	printf("\n");
	print_hand(state);
}

static int compare_card_weights(const void *p, const void *q) {
	struct card_weight card_one = *(const struct card_weight *)p;
	struct card_weight card_two = *(const struct card_weight *)q;

	if (card_one.weight > card_two.weight)
		return 1;
	if (card_one.weight < card_two.weight)
		return -1;
	return 0;
}

static void calc_card_weight(struct state *state, struct card_weight *card_weight)
{
	int piles_weight = 0; // if the thing on hand is already top of other pile, downprio
	int king_weight = 0;
	int ace_weight = 0;
	int weight = 0;
	struct card *card_p = &state->hand[card_weight->index];
	int color = card_p->color;
	int value = card_p->value;
	int king_value = state->top_of_kings[color].value;
	if (king_value == -1) king_value = 13;
	int ace_value = state->top_of_aces[color].value;
	if (value < king_value)
		king_weight = (king_value - value);
	else 
		king_weight = 14;
	if (value > ace_value)
		ace_weight = (value - ace_value);
	else
		ace_weight = 14;

	for (int i = 0; i < NBR_VALUES; i ++)
	{
		if (i == state->current_pile) // no need to compare against ourself if we are top of hand-pile
			continue;
		if (cards_are_equal(state->top_of_piles[i], *card_p)) {
			piles_weight = 2;
			break;
		}
	}


	//printf("card value: %d, king value %d, ace value %d, king_weight %d, ace_weight %d\n", value, king_value, ace_value, king_weight, ace_weight);
	; 
	card_weight->weight = king_weight+ace_weight + 3*MIN(king_weight, ace_weight) + piles_weight;

}

void calc_new_hand_order(struct state *state, int *new_hand_order)
{
	int hand_size = calc_hand_size(state);

	struct card_weight * card_weights = (struct card_weight*)calloc(sizeof(struct card_weight),hand_size);

	for (int i = 0; i < hand_size; i++)
	{
		card_weights[i].index = i;
		calc_card_weight(state, &card_weights[i]);
		//printf("Card %d weight %d\n", card_weights[i].index,  card_weights[i].weight); 

	}
	qsort(card_weights, hand_size, sizeof(*card_weights), compare_card_weights);
	for (int i = 0; i < hand_size; i++)
		new_hand_order[i] = card_weights[i].index;

exit:
	free(card_weights);
}

static void print_dream_card(struct dream_candidate *dc, int level)
{
	printf(" L%d: val %d, src %d, idx %d, unlocks %d, to ace %d\n", level,  dc->value, dc->source, dc->source_idx, dc->unlocks, dc->dream_play_is_to_ace);

}

static int calc_unlock(const struct state *state, struct dream_candidate *dc)
{
	int unlocks = -1;
	int hand_size = calc_hand_size(state);
	int unlock_ret;
	bool found = false;
	static int level = 0;
	for (int i = 0; i < NBR_VALUES; i++)
	{
		int value;
		int color;
		if (i == state->current_pile) 
			continue;
		if (cards_are_equal(state->top_of_piles[i], no_card))
			continue;

		value =state->top_of_piles[i].value;
		color =state->top_of_piles[i].color;

		if (value == (state->top_of_kings[color].value-1) || (value == 12 && (-1 == state->top_of_kings[color].value)))
		{
			struct state new_state = *state;
			new_state.top_of_kings[color] = state->top_of_piles[i];
			new_state.top_of_piles[i] = no_card;
			level++;
			unlock_ret = 1+calc_unlock(&new_state, NULL);
			level--;
			if (unlock_ret > unlocks) {
				unlocks = unlock_ret;
				found = true;
				if (dc) {
					dc->value = value;
					dc->source_idx = i;
					dc->source = SOURCE_PILE;
					dc->dream_play_is_to_ace = false;
					dc->unlocks = unlocks = unlock_ret;
				}
			}

		}
		if (value == (state->top_of_aces[color].value+1))
		{
			struct state new_state = *state;
			new_state.top_of_aces[color] = state->top_of_piles[i];
			new_state.top_of_piles[i] = no_card;
			level++;
			unlock_ret = 1+calc_unlock(&new_state, NULL);
			level--;
			if (unlock_ret > unlocks) {
				unlocks = unlock_ret;
				found = true;
				if (dc) {
					dc->value = value;
					dc->source_idx = i;
					dc->source = SOURCE_PILE;
					dc->dream_play_is_to_ace = true;
					dc->unlocks = unlocks = unlock_ret;
				}
			}
		}
	}

	for (int i = 0; i < MAX_CARDS; i++)
	{
		int value;
		int color;
		if (cards_are_equal(state->hand[i], no_card))
			break;

		value =state->hand[i].value;
		color =state->hand[i].color;
		if (value == (state->top_of_kings[color].value-1) || (value == 12 && (-1 == state->top_of_kings[color].value)))
		{
			//printf("hand %i can be played to king\n", i);

			struct state new_state = *state;
			new_state.top_of_kings[color] = state->hand[i];
			new_state.hand[i] = new_state.hand[hand_size-1];
			new_state.hand[hand_size-1] = no_card;
			level++;
			unlock_ret = calc_unlock(&new_state, NULL);
			level--;
			if (unlock_ret > unlocks) {
				unlocks = unlock_ret;
				found = true;
				if (dc) {
					dc->value = value;
					dc->source_idx = i;
					dc->source = SOURCE_HAND;
					dc->unlocks = unlocks = unlock_ret;
					dc->dream_play_is_to_ace = false;
				}
			}
		}
		if (value == (state->top_of_aces[color].value+1))
		{
			struct state new_state = *state;
			//printf("hand %i can be played to ace\n", i);
			new_state.top_of_aces[color] = state->hand[i];
			new_state.hand[i] = new_state.hand[hand_size-1];
			new_state.hand[hand_size-1] = no_card;
			level++;
			unlock_ret = calc_unlock(&new_state, NULL);
			level--;
			if (unlock_ret > unlocks){
				unlocks = unlock_ret;
				found = true;
				if (dc) {
					dc->value = value;
					dc->source_idx = i;
					dc->source = SOURCE_HAND;
					dc->unlocks = unlocks = unlock_ret;
					dc->dream_play_is_to_ace = true;
				}
			}
		}

	}

	/*
	if (found) {
		if (dc) print_dream_card(dc, level);
		else printf("L%d, unlocks %d\n", level,unlocks);
	}
	*/
	return (unlocks == -1) ? 0:unlocks;
}

bool try_play(struct state *state, struct player_action *pa)
{
	struct dream_candidate dc = {};
	calc_unlock(state, &dc);
	pa->from_index = dc.source_idx;
	if (dc.source == SOURCE_PILE){
		if (dc.dream_play_is_to_ace)
			pa->action = ACTION_PLAY_FROM_PILE_TO_ACES;
		else
			pa->action = ACTION_PLAY_FROM_PILE_TO_KINGS;
		return true;
	}

	if (dc.source == SOURCE_HAND){
		if (dc.dream_play_is_to_ace)
			pa->action = ACTION_PLAY_FROM_HAND_TO_ACES;
		else
			pa->action = ACTION_PLAY_FROM_HAND_TO_KINGS;
		return true;
	}

	return false;
}

void identify_king_ace_transfer(struct state *state)
{
	for (int i = 0; i < 4; i++)
	{
		if (state->top_of_kings[i].value == (1+state->top_of_aces[i].value)) {
			char s[100];
			do {
				printf("WARNING! it is possible to move cards between aces and kings piles! Press 'o' to proceed.\n");
				gets(s);
			} while (s[0] != 'o');
			break;

		}
	}

}




#include <stdio.h>
#include <stdlib.h>

#include "kort.h"
#include "player.h"

static struct state last_state;
static enum action last_action;
static int number_of_reorders_since_last_state_change;

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
char *symbols[4] = {
	"♣",
	"\x1B[31m♦\x1B[0m",
	"\x1B[31m♥\x1B[0m",
	"♠"
};

struct card_weight {
	int index;
	int weight;
};

char *values[NBR_VALUES] = {
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

static int calc_hand_size(struct state state)
{
	struct card *card_p;
	int i = 0;
	while (card_p = state.hand->cards[i]) {i++;};
	return i;
}

static void print_aces(struct state state)
{
	printf("\nAces: ");
	for (int i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state.top_of_aces[i], no_card))
			printf(" %s%s ", symbols[i], values[state.top_of_aces[i].value]);
		else 
			printf(" %s- ", symbols[i]);
	}
}

static void print_kings(struct state state)
{
	printf("Kings: ");
	for (int i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state.top_of_kings[i], no_card))
			printf(" %s%s ", symbols[i], values[state.top_of_kings[i].value]);
		else 
			printf(" %s- ", symbols[i]);
	}
}

static void print_piles(struct state state)
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
		if (i == state.current_pile)
			printf("hand    "); 
		else
			printf("%s%s     ", symbols[state.top_of_piles[i].color], values[state.top_of_piles[i].value]); 

	}

}

static void print_hand_not_reordered(struct state state, int *order, int order_count)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);
	int i = 0;
	while (card_p = state.hand->cards[i])
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

static void print_hand(struct state state)
{
	printf("\nHand:\n");
	print_hand_not_reordered(state, NULL, 0);
}

static void print_state(struct state state)
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

static void calc_card_weight(struct state state, struct card_weight *card_weight)
{
	int king_weight = 0;
	int ace_weight = 0;
	int weight = 0;
	struct card *card_p = state.hand->cards[card_weight->index];
	int color = card_p->color;
	int value = card_p->value;
	int king_value = state.top_of_kings[color].value;
	if (king_value == -1) king_value = 13;
	int ace_value = state.top_of_aces[color].value;
	if (value < king_value)
		king_weight = (king_value - value);
	else 
		king_weight = 14;
	if (value > ace_value)
		ace_weight = (value - ace_value);
	else
		ace_weight = 14;
	//printf("card value: %d, king value %d, ace value %d, king_weight %d, ace_weight %d\n", value, king_value, ace_value, king_weight, ace_weight);
	; 
	card_weight->weight = king_weight+ace_weight + 3*MIN(king_weight, ace_weight);

}

static void calc_new_hand_order(struct state state, int *new_hand_order)
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

	return true;
}
	
static bool try_play_pile(struct state state, struct player_action *pa)
{
	int i;
	for (i = 0; i < NBR_VALUES; i ++)
	{
		if (cards_are_equal(state.top_of_piles[i], no_card))
			continue;
		int color = state.top_of_piles[i].color;
		int value = state.top_of_piles[i].value;
		//if we can play to aces
		//printf("Value %d, king %d, ace %d\n", value, state.top_of_kings[color].value, state.top_of_aces[color].value);
		if ((cards_are_equal(state.top_of_aces[color], no_card) && value == 0) || ((state.top_of_aces[color].value + 1) == value))
		{
			pa->action = ACTION_PLAY_FROM_PILE_TO_ACES;
			pa->from_index = i;
			return true;
		}
		//if we can play to kings
		if ((cards_are_equal(state.top_of_kings[color], no_card) && value == 12) || ((state.top_of_kings[color].value - 1) == value))
		{
			pa->action = ACTION_PLAY_FROM_PILE_TO_KINGS;
			pa->from_index = i;
			return true;
		}
	}
	pa->action = ACTION_NONE;
	return false;

}
bool piles_are_equal(struct state s0, struct state s1)
{
	for (int i = 0; i < NBR_VALUES; i ++)
	{
		if (!cards_are_equal(s0.top_of_piles[i], s1.top_of_piles[i])) {
			//printf("Piles are not equal\n");
			return false;
		}
	}
	//printf("Piles are equal\n");
	return true;
}

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	printf("\e[1;1H\e[2J");
	print_state(state);
	usleep(500000);

	// if state is changed -> play
	if (!piles_are_equal(last_state, state)) {
		last_action = pa->action = ACTION_CUSTOM_2;
		//printf("State has changed, so play\n");;
	}
	// else if number of reorders less that 2 and last action was play, reorder and increase erorder
	else if (last_action == ACTION_CUSTOM_2 && number_of_reorders_since_last_state_change < 2) {
		last_action = pa->action = ACTION_CUSTOM_1;

	}
	// else if last action was reorder -> play
	//else if (last_action == ACTION_CUSTOM_1) {
	//	last_action = pa->action = ACTION_CUSTOM_2;
	//}
	else {
		last_action = pa->action = ACTION_PUT_HAND_DOWN;
		number_of_reorders_since_last_state_change = 0;
	}

	last_state = state;


	if (pa->action == ACTION_CUSTOM_2)
	{
		//Custom automatic play action. Player defined.
		//try play pile
		//printf("Trying to play\n");
		if (try_play_pile(state, pa)) {
			number_of_reorders_since_last_state_change = 0;
			return;
		}
		//printf("Failed to play");
		last_action = pa->action = ACTION_CUSTOM_1;
	}

	if (pa->action == ACTION_CUSTOM_1)
	{
		//Custom automatic reorder action. Player defined.

		//printf("Trying to reorder\n");
		pa->action = ACTION_REORDER_HAND;
		calc_new_hand_order(state, pa->new_hand_order);
		number_of_reorders_since_last_state_change++;
		return;

	}


}

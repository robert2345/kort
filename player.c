#include <stdio.h>
#include <stdlib.h>

#include "kort.h"
#include "player.h"

static int pile_index = 0;
static int to_index = 0;
static struct card last_play = no_card;
static int hand_index = -1;
static int hand_sub_index = 0;
static bool king = false;

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

static int prompt_action()
{
	char s[100];
	int i;
	for (i = 0 ; i < ACTION_NBR_OF_NONE_CUSTOM; i++)
	{
		printf(" %d) %s\n", i, action_to_string[i]);

	}
	printf(" %d) %s\n", i++, "Automatic hand reordring.");
	printf(" %d) %s\n", i++, "Auto-play.");
	printf("Which action do you want to take?\n");
	scanf("%s", s);
	enum action a = atoi(s);
	return a;

}

static int prompt_from(struct state state, int nbr_elements)
{
	char s[100];
	printf("Which index to get card from, 0 to %d?\n", nbr_elements-1);
	scanf("%s", s);
	int i= atoi(s);
	return i;

}

static int prompt_to(struct state state, int nbr_elements)
{
	char s[100];
	printf("Which index to put card in, 0 to %d?\n", nbr_elements-1);
	scanf("%s", s);
	int i= atoi(s);
	return i;

}

static bool prompt_new_hand_order(struct state state, int *new_hand_order)
{
	int hand_size = calc_hand_size(state);
	char s[100];

	for (int i = 0; i < hand_size; i++)
	{
		printf("Cards left to order:\n");
		print_hand_not_reordered(state, new_hand_order, i);
		printf("Which hand index to add to reorder list? (list currently %d long, hand size %d", i, hand_size);
		scanf("%s", s);
		int d= atoi(s);
		if (d < 0 || d >= hand_size) {
			printf("Bad index %d, max %d!\n", d, hand_size-1);
			return false;
		}
		new_hand_order[i] = d;
			
	}

	return true;
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
	// get the value and color
	// get the diff of both ace and king
	// weight is (13 - diff)^2
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

static bool calc_new_hand_order(struct state state, int *new_hand_order)
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
		printf("Value %d, king %d, ace %d\n", value, state.top_of_kings[color].value, state.top_of_aces[color].value);
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

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	print_state(state);

	pa->action = prompt_action();

	if (pa->action == ACTION_CUSTOM_1)
	{
		//Custom automatic reorder action. Player defined.

		pa->action = ACTION_REORDER_HAND;
		if (!calc_new_hand_order(state, pa->new_hand_order)) { pa->action = ACTION_NONE; }

		return;
	}

	if (pa->action == ACTION_CUSTOM_2)
	{
		//Custom automatic play action. Player defined.
		//try play pile
		if (try_play_pile(state, pa))
			return;
		//try play from hand

		return;
	}

	switch(pa->action) {
		case ACTION_PLAY_FROM_HAND_TO_KINGS:
			print_kings(state);
			print_hand(state);
			pa->from_index = prompt_from(state, hand_size);
			break;
		case ACTION_PLAY_FROM_HAND_TO_ACES:
			print_aces(state);
			print_hand(state);
			pa->from_index = prompt_from(state, hand_size);
			break;
		case ACTION_PLAY_FROM_PILE_TO_ACES:
			print_aces(state);
			print_piles(state);
			pa->from_index = prompt_from(state, NBR_VALUES);
			break;
		case ACTION_PLAY_FROM_PILE_TO_KINGS:
			print_kings(state);
			print_piles(state);
			pa->from_index = prompt_from(state, NBR_VALUES);
			break;
		case ACTION_SWAP_CARDS_IN_HAND:
			pa->from_index = prompt_from(state, hand_size);
			pa->to_index = prompt_to(state, hand_size);
			break;
		case ACTION_REORDER_HAND:
			if (!prompt_new_hand_order(state, pa->new_hand_order)) { pa->action = ACTION_NONE; }
			break;
		case ACTION_PUT_HAND_DOWN:
			break;
		default:
			pa->action = ACTION_NONE;
			break;
	}


}

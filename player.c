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

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
char *symbols[4] = {
	"♣",
	"\x1B[31m♦\x1B[0m",
	"\x1B[31m♥\x1B[0m",
	"♠"
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
	while (card_p = state.hand->cards[i++]) {};
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

static void print_hand(struct state state)
{
	struct card *card_p;
	printf("\nHand:\n");
	int i = 0;
	while (card_p = state.hand->cards[i])
	{
			printf("%d: %s%s     ", i, symbols[card_p->color], values[card_p->value]); 
			i++;
	}

	printf("\n");
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
	for (int i = 0 ; i < ACTION_NBR_OF; i++)
	{
		printf(" %d) %s\n", i, action_to_string[i]);

	}
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

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	print_state(state);

	pa->action = prompt_action();
	printf("action %d\n", pa->action);

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
		case ACTION_PUT_HAND_DOWN:
			break;
		default:
			pa->action = ACTION_NONE;
			break;
	}


}

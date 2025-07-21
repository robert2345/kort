#include <stdio.h>

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

static void print_state(struct state state)
{
	struct card *card_p;
	int i;
	printf("Aces: ");
	for (i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state.top_of_aces[i], no_card))
			printf(" %s%02d ", symbols[i], state.top_of_aces[i].value);
		else 
			printf(" %s--   ", symbols[i]);
	}
	printf("Kings: ");
	for (i = 0; i < 4; i++)
	{
		if (!cards_are_equal(state.top_of_kings[i], no_card))
			printf(" %s%02d ", symbols[i], state.top_of_kings[i].value);
		else 
			printf(" %s--   ", symbols[i]);
	}
	printf("\n");

	printf("Piles:\n");
	for (i = 0; i < NBR_VALUES; i ++)
	{
		printf("Pile %02d ", i); 

	}
	printf("\n");
	for (i = 0; i < NBR_VALUES; i ++)
	{
		if (i == state.current_pile)
			printf("hand    "); 
		else
			printf("%s%02d     ", symbols[state.top_of_piles[i].color], state.top_of_piles[i].value); 

	}

	printf("Hand:\n");
	i = 0;
	while (card_p = state.hand->cards[i++])
	{
			printf("%d: %s%02d     ", i, symbols[card_p->color], card_p->value); 
	}
	printf("\n");
}

void player_prompt_action(struct state state, struct player_action *pa)
{
	struct card *card_p;
	print_state(state);

	if(hand_index != state.current_pile) {
		hand_index = state.current_pile;
		hand_sub_index =0;
	}
	if (card_p = state.hand->cards[hand_sub_index]) {
		pa->action = ACTION_PLAY_FROM_HAND;
		pa->from_index = hand_sub_index++;
		pa->to_index = card_p->color;
		return;
	} else
	{
		pa->action = ACTION_PUT_HAND_DOWN;
		return;
	}

}

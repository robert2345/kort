#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "kort.h"
#include "player.h"
#include "player_common.h"

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
	//if (!gets(s));
	//	return ACTION_CUSTOM_2;
	gets(s);
	if (s[0] == '\0')
		return ACTION_CUSTOM_2;
	enum action a = atoi(s);
	return a;

}

static int prompt_from(struct state *state, int nbr_elements)
{
	char s[100];
	printf("Which index to get card from, 0 to %d?\n", nbr_elements-1);
	scanf("%s", s);
	int i= atoi(s);
	return i;

}

static int prompt_to(struct state *state, int nbr_elements)
{
	char s[100];
	printf("Which index to put card in, 0 to %d?\n", nbr_elements-1);
	scanf("%s", s);
	int i= atoi(s);
	return i;

}

static bool prompt_new_hand_order(struct state *state, int *new_hand_order)
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

static int prompt_count(struct state *state, int max_count)
{
	char s[100];
	printf("How many cards to move: 0 to %d?\n", max_count);
	scanf("%s", s);
	int i= atoi(s);
	return i;
}

void player_prompt_action(struct state *state, struct player_action *pa)
{
	struct card *card_p;
	int hand_size = calc_hand_size(state);

	print_state(state);

	identify_king_ace_transfer(state);
	pa->action = prompt_action();
	printf("Action %d\n", pa->action);

	if (pa->action == ACTION_CUSTOM_2)
	{
		//Custom automatic play action. Player defined.
		if (try_play(state, pa))
			return;
		pa->action = ACTION_CUSTOM_1;
	}

	if (pa->action == ACTION_CUSTOM_1)
	{
		//Custom automatic reorder action. Player defined.

		pa->action = ACTION_REORDER_HAND;

		calc_new_hand_order(state, NULL, pa->new_hand_order);

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
		case ACTION_PLAY_FROM_ACES_TO_KINGS:
			pa->from_index = prompt_from(state, 4);
			pa->count = prompt_count(state, state->top_of_aces[pa->from_index].value+1);
			break;
		case ACTION_PLAY_FROM_KINGS_TO_ACES:
			pa->from_index = prompt_from(state, 4);
			int top_value = state->top_of_kings[pa->from_index].value;
			if (top_value == -1) top_value = NBR_VALUES;
			pa->count = prompt_count(state, NBR_VALUES-top_value);
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

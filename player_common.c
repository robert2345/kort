
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	if (card_one.weight < card_two.weight)
		return 1;
	if (card_one.weight > card_two.weight)
		return -1;
	return 0;
}

static bool play(struct state *state, bool to_aces, struct card card)
{

	char color = card.color;
	if (!to_aces) {
		if ((state->top_of_kings[color].value>0) && (((card.value == (NBR_VALUES-1)) && (state->top_of_kings[color].value == -1)) || state->top_of_kings[color].value == card.value+1)) {
			state->top_of_kings[color].value = card.value;
			//printf("Play %d:%d to kings\n",card.color, card.value);
			return true;
		}
	} else {
		if (state->top_of_aces[color].value != (NBR_VALUES-1) && state->top_of_kings[color].value == card.value-1) {
			state->top_of_aces[color].value = card.value;
			//printf("Play %d:%d to aces\n",card.color, card.value);
			return true;
		}
	}
	return false;

}

static bool play_piles(struct state *state, struct card piles[NBR_VALUES][MAX_CARDS], bool to_aces)
{
	bool played = false;
	for (int j = 0 ; j < NBR_VALUES; j++)
	{
		if (j == state->current_pile)
			continue;
		// is it correct to remove the card from the pile when pretending to play? It could be re-used on aces if we have played it on kings for instance.
		if (play(state, to_aces, state->top_of_piles[j]))
		{
			if (piles) {
				state->top_of_piles[j] = piles[j][1];
				for (int k = 1; k < MAX_CARDS; k++) {
					piles[j][k-1] = piles[j][k];
					if(cards_are_equal(no_card, piles[j][k]))
						break;
				}
				//printf("Played a card SUPER pile, idx %d\n",j);

			} else {
				state->top_of_piles[j] = no_card;
				//printf("Played a card from pile, idx %d\n",j);
			}
			played = true;
		}

	}
	return played;
}

static bool play_for_weight(struct state *state, struct card piles[NBR_VALUES][MAX_CARDS], bool to_aces, struct card_weight *card_weights, int hand_size, int *weight_add)
{	
	bool played = false;
	for (int i = 0; i < hand_size; i++)
	{
		// if it can be played, "play it" and increase_weight
		if (play(state, to_aces, state->hand[card_weights[i].index]))
		{
			played = true;
			//for now, zero out the card from hand, it messes with the hand size calc so not great.
			state->hand[card_weights[i].index] = no_card;
			card_weights[i].weight+=*weight_add;
			//printf("Card %d added %d now weight %d\n", card_weights[i].index, *weight_add, card_weights[i].weight); 
			while(play_piles(state, piles, to_aces));
			/* not great because there is no clear distinction
			 * between stuff that can be played right away or stuff
			 * that can be played only because previously played
			 * stuff from hand/pile */
			i = 0;
		}
	}
	*weight_add-=1;
	return played;
}

void calc_new_hand_order(struct state *state, struct card piles[NBR_VALUES][MAX_CARDS], int *new_hand_order)
{
	int hand_size = calc_hand_size(state);
	struct state new_state = *state;

	struct card_weight * card_weights = (struct card_weight*)calloc(sizeof(struct card_weight),hand_size);
	for (int i = 0; i < hand_size; i++)
	{
		card_weights[i].index = i;
		//printf("Card %d weight %d\n", card_weights[i].index,  card_weights[i].weight); 

	}


	//lägg till ett kort på alla kings and aces
	//weight_add-=2
	//för varje kort på handen, om det går att spela och ge dem weight_add i vikt, ge alla som går att spela på detta weight_add+1 i vikt etc.
	
	for (int to_aces = 0; to_aces < 2; to_aces++)
	{
		memcpy(new_state.hand,state->hand, hand_size*sizeof(struct card));
		int weight_add = 13*4;
		for (int j = 0 ; j < NBR_VALUES; j++)
		{
			weight_add -=2;
			//printf("ONE MORE LAYER!, To aces? %d\n", to_aces);
			for (int i = 0; i < 4; i ++){
				bool inc = false;
				if (new_state.top_of_kings[i].value > 0) {
					new_state.top_of_kings[i].value--;
					inc = true;
				} else if (new_state.top_of_kings[i].value == -1)
				{
					new_state.top_of_kings[i].value = (NBR_VALUES-1);
					inc = true;
				}

				if (new_state.top_of_aces[i].value != (NBR_VALUES-1)) {
					new_state.top_of_aces[i].value++;
					inc = true;
				}
				if (!inc)
					break; // done
			}
			while(play_piles(&new_state, piles, to_aces));

			while(play_for_weight(&new_state, piles,to_aces, card_weights, hand_size, &weight_add));
		}
	}
	qsort(card_weights, hand_size, sizeof(*card_weights), compare_card_weights);
	for (int i = 0; i < hand_size; i++) {
		//printf("weight %d\n", card_weights[i].weight);
		new_hand_order[i] = card_weights[i].index;
	}

exit:
	free(card_weights);
}

static void print_dream_card(struct dream_candidate *dc, int level)
{
	printf(" L%d: val %d, src %d, idx %d, unlocks %d, to ace %d\n", level,  dc->value, dc->source, dc->source_idx, dc->unlocks, dc->dream_play_is_to_ace);

}

static int calc_unlock(const struct state *state, struct card piles[NBR_VALUES][MAX_CARDS], struct dream_candidate *dc)
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
		int unlock_inc = 2;
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
			if (piles) {
				new_state.top_of_piles[i] = piles[i][1];
				if (cards_are_equal(no_card, piles[i][1]))
					unlock_inc = 1;
				for (int j = 1; j < MAX_CARDS; j++) {
					piles[i][j-1] = piles[i][j];
					if(cards_are_equal(no_card, piles[i][j]))
						break;
				}
				//printf("Played a card SUPER pile, idx %d\n",j);

			} else {
				new_state.top_of_piles[i] = no_card;
				//printf("Played a card from pile, idx %d\n",j);
			}
			level++;
			unlock_ret = unlock_inc+calc_unlock(&new_state, piles,NULL);
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
			if (piles) {
				new_state.top_of_piles[i] = piles[i][1];
				if (cards_are_equal(no_card, piles[i][1]))
						unlock_inc = 1;
				for (int j = 1; j < MAX_CARDS; j++) {
					piles[i][j-1] = piles[i][j];
					if(cards_are_equal(no_card, piles[i][j]))
						break;
				}
				//printf("Played a card SUPER pile, idx %d\n",j);

			} else {
				new_state.top_of_piles[i] = no_card;
				//printf("Played a card from pile, idx %d\n",j);
			}
			level++;
			unlock_ret = unlock_inc+calc_unlock(&new_state, piles, NULL);
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
			unlock_ret = calc_unlock(&new_state, piles, NULL);
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
			unlock_ret = calc_unlock(&new_state, piles, NULL);
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

bool try_play(struct state *state, struct card piles[NBR_VALUES][MAX_CARDS], struct player_action *pa)
{
	struct dream_candidate dc = {};
	calc_unlock(state, piles, &dc);
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




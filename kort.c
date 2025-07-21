#include <stdio.h>
#include <stdlib.h>

#include "player.h"

// starthög
static struct ddeck init_pile;

// 13 högar
static struct ddeck piles[NBR_VALUES];

//4 ess
static struct ddeck aces[4];

//4 kungar
static struct ddeck kings[4];

// draw pile
static struct ddeck draw_pile;

struct card *get_last_card(struct ddeck *pile)
{
	int i = 1;
	for (i = 1; i<MAX_CARDS; i ++)
		if (!pile->cards[i])
			break;
	struct card * card_p = pile->cards[i-1];
	pile->cards[i-1] = NULL;
	return card_p;
}

bool cards_are_equal(struct card a, struct card b)
{
	return a.value == b.value && a.color == b.color;
}

struct card *get_first_card(struct ddeck *pile)
{
	struct card * card_p = pile->cards[0];
	int i = 1;
	for (i = 1; i<MAX_CARDS; i ++) {
		pile->cards[i-1] = pile->cards[i];
		if (!pile->cards[i])
			break;
	}
	pile->cards[i-1] = NULL;
	return card_p;
}

static struct card * put_card_last(struct ddeck *pile, struct card *card_p)
{
	for (int i = 0; i<MAX_CARDS; i ++)
		if (!pile->cards[i])
		{
			pile->cards[i] = card_p;
			return NULL;
		}
	return card_p;

}

static struct card * get_from_index(struct ddeck *pile, int index)
{
	if (index >= MAX_CARDS) return NULL;

	struct card * card_p = pile->cards[index];
	int i = index+1;
	for (; i<MAX_CARDS; i ++) {
		pile->cards[i-1] = pile->cards[i];
		if (!pile->cards[i])
			break;
	}
	return card_p;
}

static struct card * put_card_at_index(struct ddeck *pile, struct card *card_p, int index)
{
	if (index >= MAX_CARDS) return card_p;

	if (pile->cards[MAX_CARDS-1])
		return card_p;

	struct card * tmp_card_p = pile->cards[index];
	int i = MAX_CARDS-1;
	for (; i> index; i --) {
		pile->cards[i] = pile->cards[i-1];
	}
	pile->cards[i] = card_p;
	return NULL;
}

static struct card *put_card_first(struct ddeck *pile, struct card *card_p)
{
	if (pile->cards[MAX_CARDS-1])
		return card_p;

	for (int i = MAX_CARDS-1; i>0; i --){
		pile->cards[i] = pile->cards[i-1];
	}

	pile->cards[0] = card_p;

	return NULL;
}

static void print_pile (struct ddeck *pile)
{
	for (int i = 0; i<MAX_CARDS; i ++) {
		if (!pile->cards[i])
			break;
		else
			printf("Color %d value %d\n", pile->cards[i]->color, pile->cards[i]->value);
	}

}



static void init_game()
{
	struct card *card_p;
	// skapa kortlek
	int i = 0;
	for (int deck = 0 ; deck < 2; deck++) {
		for (char color = 0; color < 4; color++) {
			for  (char value = 0; value < NBR_VALUES; value++) {
				card_p = malloc(sizeof(struct card));

				card_p->value = value;
				card_p->color = color;
				put_card_last(&init_pile,card_p);
				i++;
			}
		}

	}


	// blanda init_pile
	for (i = 0; i < 4 * MAX_CARDS; i++)
	{
		int pos1 = rand()%MAX_CARDS;
		int pos2 = rand()%MAX_CARDS;
		card_p = init_pile.cards[pos1];
		init_pile.cards[pos1] = init_pile.cards[pos2];
		init_pile.cards[pos2] = card_p;
	}

	/*
	for (i = 0; i < MAX_CARDS; i++) 
	{
		printf("Color %d value %d\n", init_pile.cards[i]->color, init_pile.cards[i]->value);
	}
	*/


	int pile_index = 0;
	while (card_p = get_first_card(&init_pile))
	{
		put_card_first(&piles[pile_index], card_p);

		// if card value == i, put a card from init pile to draw pile
		if (pile_index == card_p->value) {
			card_p = get_first_card(&init_pile);
			if (card_p) {
				put_card_last(&draw_pile, card_p);
			}
		}

		pile_index = (pile_index + 1) %NBR_VALUES;
		// if this was the last pile, try to get a bonus card.
		if (pile_index == 0) {
			card_p = get_first_card(&init_pile);
			if (card_p) {
				put_card_last(&draw_pile, card_p);
			}
		}
	}

	print_pile(&draw_pile);
	//for (int p = 0; p < NBR_VALUES; p++)
	//{
		//printf("pile %d\n",p);
		//print_pile(&piles[p]);
	//}
}

struct card * play_king(struct card *card_p, int to_index)
{
	int color  = card_p->color;
	int value = card_p->value;
	struct card *king_p;
	if (color != to_index)
		return card_p;
	king_p = get_first_card(&kings[to_index]);
	if (!king_p)
	       return value == 12 ? put_card_first(&kings[to_index], card_p) : card_p;

	put_card_first(&kings[to_index], king_p);
	if ((king_p->value - 1) == value)
		return put_card_first(&kings[to_index], card_p);
	return card_p;
}


struct card * play(struct card *card_p, int to_index)
{
	struct card *ace_p;
	if (to_index >3) {
		return play_king(card_p, to_index % 4);
	} else
	{
		int color  = card_p->color;
		int value = card_p->value;
		if (color != to_index)
			return card_p;
		ace_p = get_first_card(&aces[to_index]);
		if (!ace_p) {
			return value == 0 ? put_card_first(&aces[to_index], card_p) : card_p;
		}

		put_card_first(&aces[to_index], ace_p);
		if ((ace_p->value +1) == value) {
			return put_card_first(&aces[to_index], card_p);

		}
	}
	return card_p;
}


int main(int argc, char **argv) {

	struct card *card_p;
	struct player_action pa;
	struct state state = {0};

	// init game
	init_game();
	printf("init done\n");


	while (card_p = get_last_card(&draw_pile))
	{
		state.current_pile = card_p->value;
		put_card_first(&piles[state.current_pile], card_p);
		pa.action = ACTION_NONE;
		printf("The new pile is %d\n",state.current_pile);
		while (pa.action != ACTION_PUT_HAND_DOWN) {
			state.hand = &piles[card_p->value];
			for (int i = 0 ; i < 4; i++) {
				struct card *tmp_card_p;
				tmp_card_p = aces[i].cards[0];
				state.top_of_aces[i] = tmp_card_p ? *tmp_card_p: no_card;

				tmp_card_p = kings[i].cards[0];
				state.top_of_kings[i] = tmp_card_p ? *tmp_card_p: no_card;

			}
			for (int i = 0 ; i < NBR_VALUES; i++) {
				struct card *tmp_card_p;
				tmp_card_p = piles[i].cards[0];
				state.top_of_piles[i] = tmp_card_p ? *tmp_card_p: no_card;
			}

			player_prompt_action(state, &pa);

			printf("Player action %d, from %d, to %d\n", pa.action, pa.from_index, pa.to_index);
			switch (pa.action)
			{
				case ACTION_PLAY_FROM_PILE:
					struct card *tmp_card_p;
					tmp_card_p = get_first_card(&piles[pa.from_index]);
					if (!tmp_card_p) break;
					if (tmp_card_p = play(tmp_card_p, pa.to_index))
					{
						//failed to play this card. Put back
						put_card_first(&piles[pa.from_index], tmp_card_p);
					}
					break;
				case ACTION_PLAY_FROM_HAND:
					if (tmp_card_p = get_from_index(&piles[state.current_pile], pa.from_index))
					{
						if (tmp_card_p = play(tmp_card_p, pa.to_index)) {
							tmp_card_p = put_card_at_index(&piles[state.current_pile], tmp_card_p,  pa.from_index);
							if (tmp_card_p)
								printf("Failed to add back the failed play to the pile\n");
						}



					}
					break;
				case ACTION_PUT_HAND_DOWN:
					break;
				default:
					printf("INVALID ACTION\n");
					exit(-1);
			}

		}
	}

	return 0;

}

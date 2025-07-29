#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "player.h"

static int runs_left = 1;
static int successes = 0;
static int failures = 0;
static int max_played = 0;
static int played_cards = 0;

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
	if (pile->cards[MAX_CARDS-1]) {
		printf("Pile is full?");
		return card_p;
	}

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


static void swap_indexes(struct ddeck *pile, int i1, int i2)
{
	struct card * card_p;
	if (!pile->cards[i1] || !pile->cards[i2])
		return;
	card_p = pile->cards[i1];
	pile->cards[i1]  = pile->cards[i2] ;
	pile->cards[i2]  = card_p;
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
	srand((unsigned)clock());
	for (i = 0; i < 4 * MAX_CARDS; i++)
	{
		int pos1 = rand()%MAX_CARDS;
		int pos2 = rand()%MAX_CARDS;
		card_p = init_pile.cards[pos1];
		init_pile.cards[pos1] = init_pile.cards[pos2];
		init_pile.cards[pos2] = card_p;
	}



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

}

static void clear_game()
{
	struct card *card_p;
	for (int i = 0; i < NBR_VALUES; i++)
	{
		while (card_p = get_first_card(&piles[i]))
			free(card_p);
	}
	for (int i = 0; i < 4; i++)
	{
		while (card_p = get_first_card(&kings[i]))
			free(card_p);
	}
	for (int i = 0; i < 4; i++)
	{
		while (card_p = get_first_card(&aces[i]))
			free(card_p);
	}

}

struct card * play_kings(struct card *card_p)
{
	int color  = card_p->color;
	int value = card_p->value;
	struct card *king_p;
	king_p = get_first_card(&kings[color]);
	if (!king_p)
	       return value == (NBR_VALUES-1) ? put_card_first(&kings[color], card_p) : card_p;

	put_card_first(&kings[color], king_p);
	if ((king_p->value - 1) == value)
		return put_card_first(&kings[color], card_p);
	return card_p;
}


struct card * play_aces(struct card *card_p)
{
	struct card *ace_p;
	int color  = card_p->color;
	int value = card_p->value;
	ace_p = get_first_card(&aces[color]);
	if (!ace_p) {
		return value == 0 ? put_card_first(&aces[color], card_p) : card_p;
	}

	put_card_first(&aces[color], ace_p);
	if ((ace_p->value +1) == value) {
		return put_card_first(&aces[color], card_p);

	}
	return card_p;
}

static int calc_hand_size(struct ddeck *pile)
{
	struct card *card_p;
	int i = 0;
	while (i < MAX_CARDS && (card_p = pile->cards[i])) {i++;};
	return i;
}


static void reorder_hand(struct ddeck *pile, int *new_hand_order)
{
	int order_sum = 0;
	int expected_sum = 0;
	int hand_count = calc_hand_size(pile);

	struct ddeck replacement_pile = {0};

	for (int i = 0; i < hand_count; i++)
	{
		order_sum += new_hand_order[i] + 1;
		expected_sum += i + 1;
		if (new_hand_order[i] >= MAX_CARDS)
			return;
		replacement_pile.cards[i] = pile->cards[new_hand_order[i]];
	}

	/* The new order must contain all indexes or it is bad */
	if (order_sum != expected_sum) {
		printf("New order must contain all existing indexes!\n");
		return;
	}

	*pile = replacement_pile;

}
static int count_played_cards()
{
	int res = 0;
	for (int i = 0; i < 4; i++)
	{
		for ( int j = 0 ; j < MAX_CARDS; j++) {
			if (kings[i].cards[j]) res++;
			if (aces[i].cards[j]) res++;
		}
	}
	return res;

}

static void print_results()
{
	printf("\n\n<<<<<    Game over!    >>>>>\n\n");
	for (int i = 0; i < NBR_VALUES; i++) {
		if (piles[i].cards[0]){
			printf("         You LOST!\n\n");
			failures++;
			goto out;
		}
	}

	printf("         You WON!\n\n");
	successes++;
out:
	int played_cards_this_round = count_played_cards();
	played_cards += played_cards_this_round;
	max_played = played_cards_this_round > max_played ? played_cards_this_round : max_played;
	printf("         In all you won %d times and lost %d.\n\n", successes, failures);
	printf("         In average you managed to play %d cards. At most %d.\n\n",  played_cards/(successes+failures), max_played);

	//getchar();
}

static void move_from_aces_to_kings(int color, int count)
{
	printf("%s\n", __func__);
	for (int i = 0 ; i < count; i++)
	{
		struct card *card_p = get_first_card(&aces[color]);
		if (!card_p)
			break;
		// check that there is indeed one step between the cards and that it the put succeeds
		if (((card_p->value +1) != kings[color].cards[0]->value) || 
				(card_p = put_card_first(&kings[color], card_p)))
		{
			put_card_first(&aces[color], card_p); // if something fails, put card back
			printf("Failed to put ace on king\n");
		}
	}
}

static void move_from_kings_to_aces(int color, int count)
{
	for (int i = 0 ; i < count; i++)
	{
		struct card *card_p = get_first_card(&kings[color]);
		if (!card_p)
			break;
		// check that there is indeed one step between the cards and that it the put succeeds
		if (((card_p->value -1) != aces[color].cards[0]->value) || 
				(card_p = put_card_first(&aces[color], card_p))){
			put_card_first(&kings[color], card_p); // if something fails, put card back
			printf("Failed to put king on ace\n");
		}
	}
}

static void pick_up_pile(struct state *state, int pile)
{
	for (int i = 0; i < MAX_CARDS; i++)
	{
		struct card *card_p = piles[pile].cards[i];
		if (card_p) {
			state->hand[i] = *card_p;
		} else {
			state->hand[i] = no_card;
			break; // Leave this no_card as end marker and the rest undefined
		}
	}
}

int main(int argc, char **argv) {

	struct card *card_p;
	struct player_action pa;
	struct state state = {0};
	struct card *tmp_card_p;

	if (argc > 1)
	{
		if (argv[1][1] == 's');
		{
			printf("Gathering statistics on many runs!\n");
			runs_left = 1000;
		}

	}


	while (0 <= runs_left--) {
		// init game
		init_game();

		while (card_p = get_last_card(&draw_pile))
		{
			state.current_pile = card_p->value;
			put_card_first(&piles[state.current_pile], card_p);
			pa.action = ACTION_NONE;
			while (pa.action != ACTION_PUT_HAND_DOWN) {
				pick_up_pile(&state, card_p->value);
				for (int i = 0 ; i < 4; i++) {
					tmp_card_p = aces[i].cards[0];
					state.top_of_aces[i] = tmp_card_p ? *tmp_card_p: no_card;

					tmp_card_p = kings[i].cards[0];
					state.top_of_kings[i] = tmp_card_p ? *tmp_card_p: no_card;

				}
				for (int i = 0 ; i < NBR_VALUES; i++) {
					tmp_card_p = piles[i].cards[0];
					state.top_of_piles[i] = tmp_card_p ? *tmp_card_p: no_card;
				}

				player_prompt_action(&state, &pa);

				//printf("Player action %d, from %d, to %d\n", pa.action, pa.from_index, pa.to_index);
				switch (pa.action)
				{
					case ACTION_PLAY_FROM_PILE_TO_ACES:
						if (pa.from_index >= NBR_VALUES) break;
						tmp_card_p = get_first_card(&piles[pa.from_index]);
						if (!tmp_card_p) break;
						if (tmp_card_p = play_aces(tmp_card_p))
						{
							//failed to play this card. Put back
							put_card_first(&piles[pa.from_index], tmp_card_p);
						}
						break;
					case ACTION_PLAY_FROM_PILE_TO_KINGS:
						if (pa.from_index >= NBR_VALUES) break;
						tmp_card_p = get_first_card(&piles[pa.from_index]);
						if (!tmp_card_p) break;
						if (tmp_card_p = play_kings(tmp_card_p))
						{
							//failed to play this card. Put back
							put_card_first(&piles[pa.from_index], tmp_card_p);
						}
						break;
					case ACTION_PLAY_FROM_HAND_TO_ACES:
						if (pa.from_index < MAX_CARDS && (tmp_card_p = get_from_index(&piles[state.current_pile], pa.from_index)))
						{
							if (tmp_card_p = play_aces(tmp_card_p)) {
								tmp_card_p = put_card_at_index(&piles[state.current_pile], tmp_card_p,  pa.from_index);
								if (tmp_card_p)
									printf("Failed to add back the failed play to the pile\n");
							}

						}
						break;
					case ACTION_PLAY_FROM_HAND_TO_KINGS:
						if (pa.from_index < MAX_CARDS && (tmp_card_p = get_from_index(&piles[state.current_pile], pa.from_index)))
						{
							if (tmp_card_p = play_kings(tmp_card_p)) {
								printf("Failed to play to kings\n");
								tmp_card_p = put_card_at_index(&piles[state.current_pile], tmp_card_p,  pa.from_index);
								if (tmp_card_p)
									printf("Failed to add back the failed play to the pile\n");
							}
						} else {

							printf("Unable to draw card");
						}
						break;
					case ACTION_SWAP_CARDS_IN_HAND:
						swap_indexes(&piles[state.current_pile], pa.from_index, pa.to_index);
						break;
					case ACTION_REORDER_HAND:
						reorder_hand(&piles[state.current_pile], pa.new_hand_order);
						break;
					case ACTION_NONE:
						break;
					case ACTION_PLAY_FROM_ACES_TO_KINGS:
						move_from_aces_to_kings(pa.from_index, pa.count);
						break;
					case ACTION_PLAY_FROM_KINGS_TO_ACES:
						move_from_kings_to_aces(pa.from_index, pa.count);
						break;
					case ACTION_PUT_HAND_DOWN:
						break;
					default:
						printf("INVALID ACTION\n");
						exit(-1);
				}
			}
		}
		print_results();
		clear_game();
	}
	return 0;
}

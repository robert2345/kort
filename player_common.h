#pragma once
#include "kort.h"

/* utilities */
int calc_hand_size(struct state *state);

void identify_king_ace_transfer(struct state *state);
	
/* cosmetics */
void print_aces(struct state *state);

void print_kings(struct state *state);

void print_piles(struct state *state);

void print_hand_not_reordered(struct state *state, int *order, int order_count);

void print_hand(struct state *state);

void print_state(struct state *state);

/* automation */
void calc_new_hand_order(struct state *state, int *new_hand_order);
	
bool try_play_pile(struct state *state, struct player_action *pa);

bool try_play_hand(struct state *state, struct player_action *pa);



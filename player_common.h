#pragma once
#include "kort.h"

enum src {
	SOURCE_NONE,
	SOURCE_PILE,
	SOURCE_HAND,
};

struct dream_candidate {
	enum src source;
	char value;
	char source_idx;
	char unlocks;
	bool dream_play_is_to_ace;
};


/* utilities */
int calc_hand_size(const struct state *state);

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

static int compare_dream_candidates(const void *p, const void *q) {
    struct dream_candidate dc0 = *(const struct dream_candidate *)p;
    struct dream_candidate dc1 = *(const struct dream_candidate *)q;

    if (dc0.value > dc1.value)
	    return 1;
    if (dc0.value < dc1.value)
	    return -1;
    if (dc0.unlocks > dc1.unlocks)
	    return 1;
    if (dc0.unlocks < dc1.unlocks)
	    return -1;
    return 0;
}

bool try_play(struct state *state, struct player_action *pa);


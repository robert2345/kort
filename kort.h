#pragma once

#include <stdbool.h>

#define DECK_SIZE 52
#define NBR_VALUES 13
#define MAX_CARDS (2*DECK_SIZE)

struct card {
	char value;
	char color;
};

struct ddeck {
	struct card *cards[MAX_CARDS];
};


struct state {
	int current_pile;
	struct card top_of_piles[NBR_VALUES];
	struct card top_of_aces[4];
	struct card top_of_kings[4];
	struct ddeck *hand;
};

static const struct card no_card = {.value = -1, .color = -1};

enum action {
	ACTION_NONE = 0,
	ACTION_PUT_HAND_DOWN,
	ACTION_PLAY_FROM_HAND_TO_ACES,
	ACTION_PLAY_FROM_HAND_TO_KINGS,
	ACTION_PLAY_FROM_PILE_TO_ACES,
	ACTION_PLAY_FROM_PILE_TO_KINGS,
	ACTION_PLAY_FROM_ACES_TO_KINGS,
	ACTION_PLAY_FROM_KINGS_TO_ACES,
	ACTION_SWAP_CARDS_IN_HAND,
	ACTION_NBR_OF
};

struct player_action {
	enum action action;
	char from_index;
	char to_index;
};


bool cards_are_equal(struct card a, struct card b);

static char* action_to_string[ACTION_NBR_OF] = 
{
	[ACTION_NONE] = "No action.",
	[ACTION_PUT_HAND_DOWN] = "Put hand down.",
	[ACTION_PLAY_FROM_HAND_TO_ACES] = "Play from hand to ace-pile.",
	[ACTION_PLAY_FROM_HAND_TO_KINGS] = "Play from hand to king-pile.",
	[ACTION_PLAY_FROM_PILE_TO_ACES] = "Play from top of pile to ace-pile.",
	[ACTION_PLAY_FROM_PILE_TO_KINGS] = "Play from top of pile to king-pile.",
	[ACTION_PLAY_FROM_ACES_TO_KINGS] = "Move from aces to kings.",
	[ACTION_PLAY_FROM_KINGS_TO_ACES] = "Move from kings to aces.",
	[ACTION_SWAP_CARDS_IN_HAND] = "Swap two cards on hands to re-order it"
};

#ifndef pokerh_H
#define pokerh_H

#define MAX_PLAYERS 8
#define START_MONEY 2500

#define SMALL_BLIND 50
#define BIG_BLIND 100

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int read_int(void) {
  int value = 0;

  while (1) {
    int result = scanf("%d", &value);

    if (result == 1) {
      return value;
    }

    if (result == EOF) {
      printf("\nInput closed. Auto-folding.\n");
      return 0;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }

    printf("Invalid input, enter a number: ");
  }
}

// pokerh.h
typedef enum suit { hearts, diamonds, clubs, spades } suit;
typedef enum face {
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten,
  Jack,
  Queen,
  King,
  Ace
} face;

typedef enum rank {
  High_Card,
  One_Pair,
  Two_Pair,
  Three_of_a_Kind,
  Straight,
  Flush,
  Full_House,
  Four_of_a_Kind,
  Straight_Flush,
  Royal_Flush
} rank;

typedef enum action { Call, Check, Fold, Raise, All_in } action;

typedef enum player_type { AI, Human } player_type;

typedef struct pot pot;
typedef struct pot_manager pot_manager;
typedef struct card card;
typedef struct deck deck;
typedef struct player player;
typedef struct table table;

struct card {
  face face;
  suit suit;
};

struct deck {
  card cards[52];
  int num_cards_dealt;
};

struct player {
  card hand[2];
  int cards_in_hand;
  int cash;
  player_type type;
};

struct table {
  int num_players;
  player players[MAX_PLAYERS];
  deck deck;
  card community_cards[5];
  bool active_players[MAX_PLAYERS];
  int active_players_num;
  int dealer_id;
  int small_blind_id;
  int big_blind_id;
};

typedef enum { Pre_flop, Flop, Turn, River, Showdown } poker_stage;

// deck related functions:

// initialize a brand new deck in place
void deck_init(deck *deck);

// shuffle the deck in place
void deck_shuffle(deck *deck);

// player related functions:

// init a player in place
void player_init(player *player);

// deal 1 card to a player from the deck
void player_deal_card(player *player, deck *deck);

// return both cards of a player to the deck
void player_return_cards_to_deck(player *player);

// table related functions:

// initialize a table in place
void table_init(table *table, int num_players);

// deal cards to all the active players on the table
void table_deal_players_cards(table *table);

// finds the next active player on the table
static int next_active_player(table *table, int player_id);

// reset the table and get it ready for the next hand
void table_reset(table *table);

// draw a card from deck to out
void draw_card(deck *deck, card *out);

// comparing hands logic:

typedef struct card_lexicographic_component {
  int frequency;
  card card;
} card_lexicographic_component;

// compare 2 card_lexicographic_components
int comp_card_lexicographic_component(const void *a, const void *b);

// takes 7 cards and return a number which can be compared to decide which hand
// wins/ checks every possible combination of 5 cards. (total of 21
// combinations)
uint32_t evaluate_7_cards(const card cards[7]);

// takes combination of 5 cards and make a number represent the hand
uint32_t evaluate_5_cards(const card hand[5]);

// evaluate a player's hand
uint32_t evaluate_player_hand(table *table, int player_id);

// gameplay:

// play one complete poker hand
void play_one_hand(table *table);

// run a full match until one player remains or the max hand limit is reached
void run_match(table *table, int max_hands);

// make one betting round, return false if only one player remains in the hand
bool betting_round(table *table, poker_stage stage, pot_manager *manager);

// deal the flop cards (first 3 cards in one hand)
void deal_flop_cards(table *table);

// deal the turn card (4th card)
void deal_turn_card(table *table);

// deal the river card (4th card)
void deal_river_card(table *table);

// showdown stage
void showdown(table *table);

// puts the blinds on the table
void post_blinds(table *table, pot_manager *manager);

// player contribute chips with this function. updates the player's cash,
// contibutions and round contributions
void contribute_chips(table *table, pot_manager *manager, int player_id,
                      int amount);

// pot logic:

struct pot {
  int amount;
  bool eligible_players[MAX_PLAYERS];
};

// manages the money on the table
struct pot_manager {
  // total amount each player put into this hand
  int contributions[MAX_PLAYERS];

  // amount each player put into the current betting round
  int current_round_contributions[MAX_PLAYERS];

  pot pots[MAX_PLAYERS];
  int num_pots;

  int current_bet;
};

void pot_init(pot *pot);

void pot_manager_init(pot_manager *manager);

// build all the pots inside pot_manager
void build_pots(table *table, pot_manager *manager);

// distribute money between all the players at play_one_hand
void distribute_money(table *table, pot_manager *manager);

// player's action logic

typedef struct player_decision {
  int amount;
  action action;
} player_decision;

void player_decision_set_human(table *table, pot_manager *manager,
                               int player_id, player_decision *decision);

// Simple AI: check when legal, call when needed, fold occasionally.
void player_decision_set_AI(table *table, pot_manager *manager, int player_id,
                            player_decision *decision);

void execute_decision(table *table, pot_manager *manager, int player_id,
                      player_decision decision);

bool validate_decision(table *table, pot_manager *manager, int player_id,
                       player_decision decision);

#endif // pokerh_H
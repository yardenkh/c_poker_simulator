#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pokerh.h"

void deck_init(deck *deck) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 13; j++) {
      deck->cards[i * 13 + j] = (card){.face = j, .suit = i};
    }
  }
  deck->num_cards_dealt = 0;
}

void deck_shuffle(deck *deck) {
  for (int i = 51; i > 0; i--) {
    /*
     * Pick a random index from 0 through i.
     */
    int j = rand() % (i + 1);

    /*
     * Swap cards[i] and cards[j].
     */
    card temp = deck->cards[i];
    deck->cards[i] = deck->cards[j];
    deck->cards[j] = temp;
  }
}

void player_init(player *player) {
  if (!player)
    return;
  player->cards_in_hand = 0;
  player->cash = START_MONEY;
  player->type = AI;
}

void player_deal_card(player *player, deck *deck) {
  if (!player || !deck || player->cards_in_hand >= 2 ||
      deck->num_cards_dealt >= 52)
    return;

  card card_to_deal = deck->cards[deck->num_cards_dealt];
  player->hand[player->cards_in_hand] = card_to_deal;
  player->cards_in_hand++;
  deck->num_cards_dealt++;
}

void player_return_cards_to_deck(player *player) {
  if (!player)
    return;

  player->cards_in_hand = 0;
}

void table_init(table *table, int num_players) {
  if (!table || num_players < 2 || num_players > MAX_PLAYERS)
    return;

  for (int i = 0; i < MAX_PLAYERS; i++) {
    table->active_players[i] = false;
  }

  table->num_players = num_players;

  deck_init(&table->deck);

  table->dealer_id = 0;

  if (num_players == 2) {
    table->small_blind_id = 0;
    table->big_blind_id = 1;
  } else {
    table->small_blind_id = 1;
    table->big_blind_id = 2;
  }

  for (int i = 0; i < num_players; i++) {
    player_init(&table->players[i]);
    table->active_players[i] = true;
  }
  table->active_players_num = num_players;
}

// finds the next active player on the table
static int next_active_player(table *table, int player_id) {
  do {
    player_id = (player_id + 1) % table->num_players;
  } while (!table->active_players[player_id]);

  return player_id;
}

void table_deal_players_cards(table *table) {
  if (!table)
    return;

  int player_id = table->small_blind_id;

  // deal the first card
  do {
    player_deal_card(&table->players[player_id], &table->deck);
    player_id = next_active_player(table, player_id);
  } while (player_id != table->small_blind_id);

  // deal the second card
  do {
    player_deal_card(&table->players[player_id], &table->deck);
    player_id = next_active_player(table, player_id);
  } while (player_id != table->small_blind_id);
}

void table_reset(table *table) {

  if (!table)
    return;

  table->active_players_num = 0;

  // Reset every player's hand and determine who can play the next hand.
  for (int i = 0; i < table->num_players; i++) {

    player_return_cards_to_deck(&table->players[i]);

    table->active_players[i] = table->players[i].cash > 0;
    table->active_players_num += table->players[i].cash > 0;
  }

  // Reset the community cards.
  memset(table->community_cards, 0, sizeof(table->community_cards));

  // Start dealing from the beginning of the deck on the next hand.
  table->deck.num_cards_dealt = 0;

  // The game is over if fewer than 2 players still have money.
  if (table->active_players_num < 2)
    return;

  // Move the dealer button to the next player who still has money.
  table->dealer_id = next_active_player(table, table->dealer_id);

  // Heads-up poker:
  // the dealer is also the small blind.
  if (table->active_players_num == 2) {

    table->small_blind_id = table->dealer_id;

    table->big_blind_id = next_active_player(table, table->small_blind_id);

    return;
  }

  // 3+ players:
  // dealer -> small blind -> big blind.
  table->small_blind_id = next_active_player(table, table->dealer_id);

  table->big_blind_id = next_active_player(table, table->small_blind_id);
}

void draw_card(deck *deck, card *out) {
  if (!deck || !out)
    return;

  // No cards left in the deck.
  if (deck->num_cards_dealt >= 52)
    return;

  *out = deck->cards[deck->num_cards_dealt++];
}

static const char *face_to_string(face value) {
  switch (value) {
  case Two:
    return "2";
  case Three:
    return "3";
  case Four:
    return "4";
  case Five:
    return "5";
  case Six:
    return "6";
  case Seven:
    return "7";
  case Eight:
    return "8";
  case Nine:
    return "9";
  case Ten:
    return "10";
  case Jack:
    return "Jack";
  case Queen:
    return "Queen";
  case King:
    return "King";
  case Ace:
    return "Ace";
  default:
    return "Unknown";
  }
}

static const char *suit_to_string(suit value) {
  switch (value) {
  case hearts:
    return "hearts";
  case diamonds:
    return "diamonds";
  case clubs:
    return "clubs";
  case spades:
    return "spades";
  default:
    return "unknown";
  }
}

static void print_card(card card_value) {
  printf("%s %s", face_to_string(card_value.face),
         suit_to_string(card_value.suit));
}

static void print_cards_list(const card cards[], int count) {
  if (count <= 0) {
    printf("none");
    return;
  }

  for (int i = 0; i < count; i++) {
    if (i > 0)
      printf(", ");
    print_card(cards[i]);
  }
}

static void print_human_turn_view(table *table, pot_manager *manager,
                                  int player_id) {
  if (!table || !manager)
    return;

  int total_pot = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    total_pot += manager->contributions[i];
  }

  printf("\n=== Your turn, Player %d ===\n", player_id);
  printf("Your stack: %d chips\n", table->players[player_id].cash);
  printf("Pot on the table: %d chips\n", total_pot);
  printf("Current bet to call: %d\n", manager->current_bet);
  printf("Your hand: ");
  print_cards_list(table->players[player_id].hand, 2);
  printf("\n");

  printf("Community cards: ");
  int visible_community = 0;
  for (int i = 0; i < 5; i++) {
    if (table->community_cards[i].face == 0 &&
        table->community_cards[i].suit == 0)
      break;
    visible_community++;
  }
  print_cards_list(table->community_cards, visible_community);
  printf("\n");

  printf("Table status:\n");
  for (int i = 0; i < table->num_players; i++) {
    if (i == player_id)
      continue;

    if (!table->active_players[i]) {
      printf("  Player %d: folded\n", i);
      continue;
    }

    printf("  Player %d: %d chips\n", i, table->players[i].cash);
  }
}

static void log_action(table *table, int player_id, player_decision decision) {
  if (!table)
    return;

  switch (decision.action) {
  case Fold:
    if (player_id == 0) {
      printf("You fold.\n");
    } else {
      printf("Player %d folds.\n", player_id);
    }
    break;
  case Check:
    if (player_id == 0) {
      printf("You check.\n");
    } else {
      printf("Player %d checks.\n", player_id);
    }
    break;
  case Call:
    if (player_id == 0) {
      printf("You call.\n");
    } else {
      printf("Player %d calls.\n", player_id);
    }
    break;
  case Raise:
    if (player_id == 0) {
      printf("You raise by %d.\n", decision.amount);
    } else {
      printf("Player %d raises by %d.\n", player_id, decision.amount);
    }
    break;
  case All_in:
    if (player_id == 0) {
      printf("You go all-in.\n");
    } else {
      printf("Player %d goes all-in.\n", player_id);
    }
    break;
  default:
    break;
  }
}

uint32_t evaluate_7_cards(const card cards[7]) {
  uint32_t best = 0;

  for (int a = 0; a < 7; a++) {
    for (int b = a + 1; b < 7; b++) {
      card hand[5];
      int k = 0;

      for (int i = 0; i < 7; i++) {
        if (i != a && i != b)
          hand[k++] = cards[i];
      }

      // now hand contain the 5 cards, i take every possible 5 card combination
      // now i need to do something like uint32_t score = evaluate_5_cards(hand)
      uint32_t score = evaluate_5_cards(hand);

      if (score > best)
        best = score;
    }
  }

  return best;
}

int comp_card_lexicographic_component(const void *a, const void *b) {
  const card_lexicographic_component *ca = a;
  const card_lexicographic_component *cb = b;

  if (ca->frequency != cb->frequency)
    return cb->frequency - ca->frequency;

  return cb->card.face - ca->card.face;
}

uint32_t evaluate_5_cards(const card hand[5]) {
  int count_cards[13] = {0};
  for (int i = 0; i < 5; i++) {
    count_cards[hand[i].face]++;
  }

  card_lexicographic_component sorted_cards[5];

  for (int i = 0; i < 5; i++) {
    sorted_cards[i] = (card_lexicographic_component){
        .frequency = count_cards[hand[i].face], .card = hand[i]};
  }

  qsort(sorted_cards, 5, sizeof(sorted_cards[0]),
        comp_card_lexicographic_component);

  uint32_t score = 0;

  // now check for the hand's rank:

  rank hand_rank;
  bool is_flush = true;
  bool is_straight = true; // includes ace high straight

  for (int i = 1; i < 5; i++) {
    if (sorted_cards[i].card.suit != sorted_cards[i - 1].card.suit)
      is_flush = false;

    if (sorted_cards[i].card.face + 1 != sorted_cards[i - 1].card.face)
      is_straight = false;
  }

  bool is_5_high_straight =
      (sorted_cards[0].card.face == Ace && sorted_cards[1].card.face == Five &&
       sorted_cards[2].card.face == Four &&
       sorted_cards[3].card.face == Three && sorted_cards[4].card.face == Two);

  // Royal_Flush is automatically taken care of by the rest of the function.
  // no need to implement a special section for it.

  // Straight_Flush:
  if (is_flush && (is_straight || is_5_high_straight)) {
    hand_rank = Straight_Flush;
  }

  // Four_of_a_Kind:
  // since the cards are sorted first by frequency, the only option is: [x, x,
  // x, x, y] so just check the frequency of the card in the middle
  else if (sorted_cards[2].frequency == 4) {
    hand_rank = Four_of_a_Kind;
  }

  // Full_House
  // it's gotta look like this: [x, x, x, y, y] so check the card in the
  // middle and the one after
  else if (sorted_cards[2].frequency == 3 && sorted_cards[3].frequency == 2) {
    hand_rank = Full_House;
  }

  // Flush
  else if (is_flush) {
    hand_rank = Flush;
  }

  // Straight
  else if (is_straight || is_5_high_straight) {
    hand_rank = Straight;
  }

  // Three_of_a_Kind
  // [x, x, x, y, z]
  else if (sorted_cards[2].frequency == 3) {
    hand_rank = Three_of_a_Kind;
  }

  // Two_Pair
  // [x, x, y, y, z]
  else if (sorted_cards[1].frequency == 2 && sorted_cards[3].frequency == 2) {
    hand_rank = Two_Pair;
  }

  // One_Pair
  else if (sorted_cards[1].frequency == 2) {
    hand_rank = One_Pair;
  }

  // High_Card
  else {
    hand_rank = High_Card;
  }

  score |= ((uint32_t)hand_rank << 20);

  // straights only need their highest card.
  // a 5 high straight is the only special case because ace acts as the low
  // card.
  if (hand_rank == Straight || hand_rank == Straight_Flush) {
    face to_shift = is_5_high_straight ? Five : sorted_cards[0].card.face;
    score |= ((uint32_t)to_shift << 16);
  }

  // for every other hand, the frequency sort already arranged the cards
  // in the exact order needed for comparison.
  else {
    for (int i = 0; i < 5; i++) {
      score |= ((uint32_t)sorted_cards[i].card.face << (16 - i * 4));
    }
  }

  return score;
}

void play_one_hand(table *table) {

  // initialize the pot_manager
  pot_manager manager;
  pot_manager_init(&manager);

  // 1. shuffle the deck
  deck_shuffle(&table->deck);

  // 2. post the blinds
  post_blinds(table, &manager);

  // 3. deal cards to players in the right order
  table_deal_players_cards(table);

  // 4. pre-flop betting round
  bool resume_hand = betting_round(table, Pre_flop, &manager);

  if (!resume_hand) {
    build_pots(table, &manager);
    distribute_money(table, &manager);
    table_reset(table);
    return;
  }

  // there are at least 2 people taking part in the hand.

  // 5. now the dealer will deal a flop, 3 face up community cards
  deal_flop_cards(table);

  // followed by a second betting round
  resume_hand = betting_round(table, Flop, &manager);

  if (!resume_hand) {
    build_pots(table, &manager);
    distribute_money(table, &manager);
    table_reset(table);
    return;
  }

  // 6. a single community card called turn is dealt
  deal_turn_card(table);

  // followed by a third betting round
  resume_hand = betting_round(table, Turn, &manager);

  if (!resume_hand) {
    build_pots(table, &manager);
    distribute_money(table, &manager);
    table_reset(table);
    return;
  }

  // 7. a single community card called river is dealt
  deal_river_card(table);

  // followed by a 4th betting round
  resume_hand = betting_round(table, River, &manager);

  if (!resume_hand) {
    build_pots(table, &manager);
    distribute_money(table, &manager);
    table_reset(table);
    return;
  }

  // 8. showdown
  showdown(table);
  build_pots(table, &manager);
  distribute_money(table, &manager);
  table_reset(table);
}

void pot_init(pot *pot) {
  pot->amount = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    pot->eligible_players[i] = false;
  }
}

void pot_manager_init(pot_manager *manager) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    pot_init(&manager->pots[i]);
    manager->contributions[i] = 0;
    manager->current_round_contributions[i] = 0;
  }
  manager->num_pots = 0;
  manager->current_bet = 0;
}

void post_blinds(table *table, pot_manager *manager) {
  int small_blind_id = table->small_blind_id;
  int big_blind_id = table->big_blind_id;

  contribute_chips(table, manager, small_blind_id, SMALL_BLIND);
  contribute_chips(table, manager, big_blind_id, BIG_BLIND);

  manager->current_bet = BIG_BLIND;
}

void contribute_chips(table *table, pot_manager *manager, int player_id,
                      int amount) {

  // Contributions must always be positive.
  if (amount <= 0)
    return;

  // A player can never contribute more money than they currently have.
  amount = MIN(amount, table->players[player_id].cash);

  table->players[player_id].cash -= amount;

  // Tracks everything this player has put into the entire hand.
  manager->contributions[player_id] += amount;

  // Tracks only what this player has put into the current betting round.
  manager->current_round_contributions[player_id] += amount;
}

bool betting_round(table *table, poker_stage stage, pot_manager *manager) {
  int offset;

  if (stage != Pre_flop) {
    manager->current_bet = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
      manager->current_round_contributions[i] = 0;
    }
    offset = table->dealer_id + 1;
  } else {
    offset = table->big_blind_id + 1;
  }

  // here i'm checking if the hand needs to end
  if (table->active_players_num <= 1) {
    return false;
  }

  int decisions_left = table->active_players_num;
  int player_counter = offset % table->num_players;

  while (decisions_left) {

    // Folded players are not included in active_players_num.
    if (!table->active_players[player_counter]) {
      player_counter = (player_counter + 1) % table->num_players;
      continue;
    }

    // All-in players are still active, but they cannot make a decision.
    if (table->players[player_counter].cash == 0) {
      decisions_left--;
      player_counter = (player_counter + 1) % table->num_players;
      continue;
    }
    while (1) {
      player_decision decision = {.action = Fold, .amount = 0};

      if (table->players[player_counter].type == Human) {
        player_decision_set_human(table, manager, player_counter, &decision);
      } else if (table->players[player_counter].type == AI) {
        player_decision_set_AI(table, manager, player_counter, &decision);
        printf("Player %d: ", player_counter);
        if (decision.action == Fold) {
          printf("folds\n");
        } else if (decision.action == Check) {
          printf("checks\n");
        } else if (decision.action == Call) {
          printf("calls\n");
        } else if (decision.action == Raise) {
          printf("raises by %d\n", decision.amount);
        } else if (decision.action == All_in) {
          printf("goes all-in\n");
        }
      }

      if (!validate_decision(table, manager, player_counter, decision)) {
        printf("Invalid move for the current bet state. Try again.\n");
        continue;
      }

      // Save the current bet so we can detect whether this action
      // increased the bet, including an all-in raise.
      int previous_bet = manager->current_bet;

      execute_decision(table, manager, player_counter, decision);
      log_action(table, player_counter, decision);

      // If everyone except one player folded, the hand is over.
      if (table->active_players_num <= 1)
        return false;

      // If the bet increased, every other active player must respond again.
      if (manager->current_bet > previous_bet) {
        decisions_left = table->active_players_num - 1;
      } else {
        decisions_left--;
      }

      break;
    }

    player_counter = (player_counter + 1) % table->num_players;
  }

  return true;
}

/*
 * Burns one card from the deck.
 *
 * In Texas Hold'em one card is burned before dealing the flop,
 * turn and river.
 */
static void burn_card(table *table) {

  card burned_card;

  draw_card(&table->deck, &burned_card);
}

// Deal the flop cards (first 3 community cards).
void deal_flop_cards(table *table) {

  burn_card(table);

  for (int i = 0; i < 3; i++) {
    draw_card(&table->deck, &table->community_cards[i]);
  }
}

// Deal the turn card (4th community card).
void deal_turn_card(table *table) {

  burn_card(table);

  draw_card(&table->deck, &table->community_cards[3]);
}

// Deal the river card (5th community card).
void deal_river_card(table *table) {

  burn_card(table);

  draw_card(&table->deck, &table->community_cards[4]);
}

/*
 * Showdown currently doesn't need to modify the game state.
 *
 * distribute_money() evaluates the eligible players and determines
 * who wins each pot.
 *
 * This function exists as a separate stage so later it can be used
 * for things such as revealing cards or printing showdown information.
 */
void showdown(table *table) {
  if (!table)
    return;

  printf("Showdown:\n");
  for (int i = 0; i < table->num_players; i++) {
    if (!table->active_players[i])
      continue;

    printf("  Player %d: ", i);
    for (int j = 0; j < 2; j++) {
      card c = table->players[i].hand[j];
      print_card(c);
      if (j == 0)
        printf(" and ");
    }
    printf(" | score=%u\n", evaluate_player_hand(table, i));
  }
}

static void print_round_state(table *table) {
  if (!table)
    return;

  printf("Table state:\n");
  for (int i = 0; i < table->num_players; i++) {
    printf("  Player %d: cash=%d active=%s\n", i, table->players[i].cash,
           table->active_players[i] ? "yes" : "no");
  }
  printf("  Community cards: ");
  int visible_community = 0;
  for (int i = 0; i < 5; i++) {
    if (table->community_cards[i].face == 0 &&
        table->community_cards[i].suit == 0) {
      break;
    }
    if (visible_community > 0)
      printf(", ");
    print_card(table->community_cards[i]);
    visible_community++;
  }
  if (visible_community == 0)
    printf("none");
  printf("\n");
}

void player_decision_set_human(table *table, pot_manager *manager,
                               int player_id, player_decision *decision) {
  if (!table || !manager || !decision)
    return;

  int amount_to_call =
      manager->current_bet - manager->current_round_contributions[player_id];
  if (amount_to_call < 0)
    amount_to_call = 0;

  while (true) {
    print_human_turn_view(table, manager, player_id);
    printf("\nChoose one:\n");
    printf("  1) Fold\n");
    if (amount_to_call == 0) {
      printf("  2) Check\n");
    } else {
      printf("  2) Call (%d)\n", amount_to_call);
    }
    printf("  3) Raise\n");
    printf("  4) All-in\n");
    printf("Your choice: ");

    int action = read_int();

    switch (action) {
    case 1:
      decision->action = Fold;
      decision->amount = 0;
      return;

    case 2:
      if (amount_to_call == 0) {
        decision->action = Check;
        decision->amount = 0;
        return;
      }
      decision->action = Call;
      decision->amount = 0;
      return;

    case 3:
      decision->action = Raise;
      printf("Raise amount: ");
      do {
        decision->amount = read_int();
        if (decision->amount <= 0) {
          printf("Raise amount must be positive: ");
        }
      } while (decision->amount <= 0);
      return;

    case 4:
      decision->action = All_in;
      decision->amount = 0;
      return;

    default:
      printf("Please enter 1, 2, 3, or 4.\n");
    }
  }
}

void player_decision_set_AI(table *table, pot_manager *manager, int player_id,
                            player_decision *decision) {
  if (!table || !manager || !decision)
    return;

  int amount_to_call =
      manager->current_bet - manager->current_round_contributions[player_id];

  if (amount_to_call <= 0) {
    decision->action = Check;
    decision->amount = 0;
    return;
  }

  if (rand() % 10 == 0) {
    decision->action = Fold;
    decision->amount = 0;
    return;
  }

  decision->action = Call;
  decision->amount = 0;
}

void execute_decision(table *table, pot_manager *manager, int player_id,
                      player_decision decision) {

  action player_action = decision.action;

  switch (player_action) {

  case Fold:
    // The player's previously contributed money stays in the pot,
    // but the player can no longer win any pot.
    table->active_players[player_id] = false;
    table->active_players_num--;
    break;

  case Check:
    // Nothing needs to be done.
    // validate_decision() should already have verified that checking is legal.
    break;

  case Call: {
    // Calculate how much more this player needs to contribute
    // to match the current bet.
    int amount_to_call =
        manager->current_bet - manager->current_round_contributions[player_id];

    // If the player cannot fully call, they call all-in instead.
    amount_to_call = MIN(amount_to_call, table->players[player_id].cash);

    contribute_chips(table, manager, player_id, amount_to_call);
    break;
  }

  case Raise: {
    /*
     * decision.amount is the amount by which the player raises.
     *
     * Example:
     * current_bet = 50
     * player's current contribution = 25
     * decision.amount = 30
     *
     * New bet = 80
     * Player must contribute 80 - 25 = 55.
     */
    int new_bet = manager->current_bet + decision.amount;

    int amount_to_contribute =
        new_bet - manager->current_round_contributions[player_id];

    contribute_chips(table, manager, player_id, amount_to_contribute);

    manager->current_bet = new_bet;
    break;
  }

  case All_in: {
    // The player contributes all remaining chips.
    int amount = table->players[player_id].cash;

    contribute_chips(table, manager, player_id, amount);

    /*
     * An all-in can increase the current bet.
     *
     * Example:
     * current_bet = 50
     * player already contributed 20
     * player has 70 remaining
     *
     * After going all-in, their round contribution is 90,
     * so the current bet becomes 90.
     */
    if (manager->current_round_contributions[player_id] >
        manager->current_bet) {

      manager->current_bet = manager->current_round_contributions[player_id];
    }

    break;
  }
  }
}

void build_pots(table *table, pot_manager *manager) {
  manager->num_pots = 0;

  int previous_level = 0;

  while (1) {
    int next_level = -1;

    // Find the smallest contribution greater than previous_level.
    for (int i = 0; i < MAX_PLAYERS; i++) {
      int contribution = manager->contributions[i];

      if (contribution > previous_level &&
          (next_level == -1 || contribution < next_level)) {
        next_level = contribution;
      }
    }

    // No more contribution levels.
    if (next_level == -1)
      break;

    pot *current_pot = &manager->pots[manager->num_pots];

    current_pot->amount = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
      current_pot->eligible_players[i] = false;

      /*
       * Every player who contributed at least next_level contributes
       * this layer to the pot.
       */
      if (manager->contributions[i] >= next_level) {
        current_pot->amount += next_level - previous_level;

        /*
         * A player can win this pot only if they contributed enough
         * and they haven't folded.
         */
        if (table->active_players[i]) {
          current_pot->eligible_players[i] = true;
        }
      }
    }

    manager->num_pots++;
    previous_level = next_level;
  }
}

bool validate_decision(table *table, pot_manager *manager, int player_id,
                       player_decision decision) {

  int player_cash = table->players[player_id].cash;

  int player_contribution = manager->current_round_contributions[player_id];

  int amount_to_call = manager->current_bet - player_contribution;

  switch (decision.action) {

  case Fold:
    // Folding is always legal while the player is still in the hand.
    return true;

  case Check:
    // A player can check only if there is nothing left for them to call.
    return amount_to_call == 0;

  case Call:
    // Calling only makes sense if there is currently a bet to match.
    //
    // The player does not need enough cash for a full call:
    // if they have less, execute_decision() will make it an all-in call.
    return amount_to_call > 0 && player_cash > 0;

  case Raise: {
    // A raise must actually increase the current bet.
    if (decision.amount <= 0)
      return false;

    /*
     * The player must first call the current bet and then add the raise.
     *
     * Example:
     *
     * current_bet = 50
     * player already contributed = 20
     * raise amount = 30
     *
     * player needs:
     * (50 - 20) + 30 = 60 chips
     */
    int amount_needed = amount_to_call + decision.amount;

    // For now, require enough cash to perform the entire raise.
    // Short all-in raises can be handled through the All_in action.
    return player_cash >= amount_needed;
  }

  case All_in:
    // Going all-in is valid as long as the player still has chips.
    return player_cash > 0;
  }

  return false;
}

uint32_t evaluate_player_hand(table *table, int player_id) {
  card cards[7];
  cards[0] = table->players[player_id].hand[0];
  cards[1] = table->players[player_id].hand[1];
  for (int i = 0; i < 5; i++) {
    cards[i + 2] = table->community_cards[i];
  }
  return evaluate_7_cards(cards);
}

void distribute_money(table *table, pot_manager *manager) {

  for (int pot_id = 0; pot_id < manager->num_pots; pot_id++) {
    pot *current_pot = &manager->pots[pot_id];

    int num_eligible = 0;
    int only_eligible_player = -1;

    // Count how many players are eligible to win this pot.
    for (int player_id = 0; player_id < MAX_PLAYERS; player_id++) {
      if (!current_pot->eligible_players[player_id])
        continue;

      num_eligible++;
      only_eligible_player = player_id;
    }

    // If only one player is eligible, everyone else folded.
    // No hand evaluation is needed.
    if (num_eligible == 1) {
      table->players[only_eligible_player].cash += current_pot->amount;
      continue;
    }

    // This should never happen if the pots were built correctly.
    if (num_eligible == 0)
      continue;

    uint32_t best_score = 0;
    bool winners[MAX_PLAYERS] = {false};
    int num_winners = 0;

    // Find the best hand among the players eligible for this pot.
    for (int player_id = 0; player_id < MAX_PLAYERS; player_id++) {
      if (!current_pot->eligible_players[player_id])
        continue;

      uint32_t score = evaluate_player_hand(table, player_id);

      if (num_winners == 0 || score > best_score) {
        best_score = score;
        num_winners = 1;

        // A new best hand was found, so clear the previous winners.
        for (int i = 0; i < MAX_PLAYERS; i++) {
          winners[i] = false;
        }

        winners[player_id] = true;

      } else if (score == best_score) {
        winners[player_id] = true;
        num_winners++;
      }
    }

    // Split the pot equally between all winners.
    int money_per_winner = current_pot->amount / num_winners;
    int remaining_money = current_pot->amount % num_winners;

    int winner_ids[MAX_PLAYERS];
    int winner_count = 0;

    for (int player_id = 0; player_id < MAX_PLAYERS; player_id++) {
      if (!winners[player_id])
        continue;

      winner_ids[winner_count++] = player_id;
    }

    for (int i = 0; i < winner_count; i++) {
      int player_id = winner_ids[i];
      table->players[player_id].cash += money_per_winner;

      // Give the remaining chips one at a time in a deterministic order.
      // This avoids biasing the extras toward whichever winner happened to be
      // iterated first in a previous implementation.
      if (remaining_money > 0) {
        table->players[player_id].cash++;
        remaining_money--;
      }
    }
  }
}

void run_match(table *table, int max_hands) {
  if (!table)
    return;

  int hand_number = 0;

  while (table->active_players_num >= 2 &&
         (max_hands <= 0 || hand_number < max_hands)) {
    printf("\n=== Hand %d ===\n", hand_number + 1);
    play_one_hand(table);
    hand_number++;

    print_round_state(table);

    int active_count = 0;
    for (int i = 0; i < table->num_players; i++) {
      if (table->players[i].cash > 0)
        active_count++;
    }

    printf("Active players after hand: %d\n", active_count);
    for (int i = 0; i < table->num_players; i++) {
      printf("  Player %d: cash=%d\n", i, table->players[i].cash);
    }

    if (active_count <= 1)
      break;
  }

  int active_count = 0;
  int winner = -1;
  for (int i = 0; i < table->num_players; i++) {
    if (table->players[i].cash > 0) {
      active_count++;
      winner = i;
    }
  }

  if (active_count == 1) {
    printf("\nWinner: Player %d with %d chips\n", winner,
           table->players[winner].cash);
  } else if (active_count > 1) {
    printf("\nMatch ended by hand limit with %d active players.\n",
           active_count);
  } else {
    printf("\nAll players busted out.\n");
  }
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  srand((unsigned)time(NULL));

  table table;
  table_init(&table, 8);

  for (int i = 0; i < table.num_players; i++) {
    table.players[i].type = (i == 0) ? Human : AI;
  }

  printf("Welcome to the poker table.\n");
  printf("You are Player 0. The other 7 seats are AI opponents.\n");
  printf("The game starts with %d players at the table.\n", table.num_players);
  printf(
      "You can fold, check, call, raise, or go all-in when it is your turn.\n");

  run_match(&table, 200);

  return 0;
}
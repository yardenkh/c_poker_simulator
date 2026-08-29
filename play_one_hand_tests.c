#include <assert.h>
#include <stdio.h>

#define main poker_main
#include "poker.c"
#undef main

static void test_play_one_hand_two_players(void) {
  table table;
  table_init(&table, 2);
  srand(42);

  play_one_hand(&table);

  assert(table.num_players == 2);
  assert(table.deck.num_cards_dealt == 0);
  assert(table.players[0].cards_in_hand == 0);
  assert(table.players[1].cards_in_hand == 0);
  assert(table.active_players_num >= 1);
  assert(table.active_players_num <= 2);
  assert(table.players[0].cash >= 0);
  assert(table.players[1].cash >= 0);
}

static void test_play_one_hand_three_players(void) {
  table table;
  table_init(&table, 3);
  srand(1234);

  play_one_hand(&table);

  assert(table.num_players == 3);
  assert(table.deck.num_cards_dealt == 0);
  for (int i = 0; i < table.num_players; i++) {
    assert(table.players[i].cards_in_hand == 0);
    assert(table.players[i].cash >= 0);
  }
  assert(table.active_players_num >= 1);
  assert(table.active_players_num <= 3);
}

static void test_play_one_hand_keeps_table_consistent(void) {
  table table;
  table_init(&table, 4);
  srand(99);

  play_one_hand(&table);

  for (int i = 0; i < table.num_players; i++) {
    assert(table.players[i].cards_in_hand == 0);
    assert(table.players[i].cash >= 0);
    assert(table.players[i].cash <= START_MONEY * 2);
  }

  assert(table.deck.num_cards_dealt == 0);
}

static void test_odd_chip_pot_split_is_deterministic(void) {
  table table = {0};
  pot_manager manager = {0};

  table.players[0].cash = 0;
  table.players[1].cash = 0;
  table.active_players[0] = true;
  table.active_players[1] = true;

  manager.num_pots = 1;
  manager.pots[0].amount = 3;
  manager.pots[0].eligible_players[0] = true;
  manager.pots[0].eligible_players[1] = true;

  distribute_money(&table, &manager);

  assert(table.players[0].cash == 2);
  assert(table.players[1].cash == 1);
}

static void test_build_pots_handles_side_pots(void) {
  table table = {0};
  pot_manager manager = {0};

  for (int i = 0; i < 3; i++) {
    table.active_players[i] = true;
  }

  manager.contributions[0] = 200;
  manager.contributions[1] = 100;
  manager.contributions[2] = 50;

  build_pots(&table, &manager);

  assert(manager.num_pots == 3);
  assert(manager.pots[0].amount == 150);
  assert(manager.pots[1].amount == 100);
  assert(manager.pots[2].amount == 100);
}

static void test_folded_player_contribution_does_not_win_pot(void) {
  table table = {0};
  pot_manager manager = {0};

  table.active_players[0] = true;
  table.active_players[1] = true;
  table.active_players[2] = false;

  manager.contributions[0] = 100;
  manager.contributions[1] = 100;
  manager.contributions[2] = 100;

  build_pots(&table, &manager);

  assert(manager.num_pots == 1);
  assert(manager.pots[0].amount == 300);
  assert(manager.pots[0].eligible_players[0] == true);
  assert(manager.pots[0].eligible_players[1] == true);
  assert(manager.pots[0].eligible_players[2] == false);
}

static void test_tied_pot_splits_evenly(void) {
  table table = {0};
  pot_manager manager = {0};

  table.active_players[0] = true;
  table.active_players[1] = true;
  table.active_players[2] = false;

  manager.num_pots = 1;
  manager.pots[0].amount = 8;
  manager.pots[0].eligible_players[0] = true;
  manager.pots[0].eligible_players[1] = true;

  distribute_money(&table, &manager);

  assert(table.players[0].cash == 4);
  assert(table.players[1].cash == 4);
}

static void test_multiple_all_in_stack_sizes_split_side_pots_correctly(void) {
  table table = {0};
  pot_manager manager = {0};

  table.active_players[0] = true;
  table.active_players[1] = true;
  table.active_players[2] = true;

  manager.contributions[0] = 100;
  manager.contributions[1] = 200;
  manager.contributions[2] = 50;

  build_pots(&table, &manager);

  assert(manager.num_pots == 3);
  assert(manager.pots[0].amount == 150);
  assert(manager.pots[1].amount == 100);
  assert(manager.pots[2].amount == 100);
}

static void test_match_runs_multiple_hands_without_breaking_state(void) {
  table table;
  table_init(&table, 4);

  for (int hand = 0; hand < 20; hand++) {
    play_one_hand(&table);

    assert(table.num_players == 4);
    assert(table.deck.num_cards_dealt == 0);

    for (int i = 0; i < table.num_players; i++) {
      assert(table.players[i].cards_in_hand == 0);
      assert(table.players[i].cash >= 0);
      assert(table.players[i].cash <= START_MONEY * 2);
    }

    assert(table.active_players_num >= 0);
    assert(table.active_players_num <= table.num_players);
  }
}

int main(void) {
  test_play_one_hand_two_players();
  test_play_one_hand_three_players();
  test_play_one_hand_keeps_table_consistent();
  test_odd_chip_pot_split_is_deterministic();
  test_build_pots_handles_side_pots();
  test_folded_player_contribution_does_not_win_pot();
  test_tied_pot_splits_evenly();
  test_multiple_all_in_stack_sizes_split_side_pots_correctly();
  test_match_runs_multiple_hands_without_breaking_state();

  puts("all play_one_hand tests passed");
  return 0;
}

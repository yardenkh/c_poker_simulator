# Poker Project TODO

## Current State

- [x] Deck creation
- [x] Deck shuffling
- [x] Card drawing
- [x] Player initialization
- [x] Table initialization
- [x] Deal hole cards
- [x] Deal flop
- [x] Deal turn
- [x] Deal river
- [x] Burn cards
- [x] 5-card hand evaluator
- [x] 7-card hand evaluator
- [x] Player hand evaluator
- [x] Post small blind / big blind
- [x] Track total hand contributions
- [x] Track current betting-round contributions
- [x] Track current bet
- [x] Basic action validation
- [x] Basic action execution
- [x] Build main pot + side pots
- [x] Distribute pots at showdown
- [x] Split tied pots
- [x] Reset table after a hand
- [x] Skip folded players
- [x] Skip all-in players when asking for an action

# 1. Finish Betting Round Logic

This is the main thing to work on next.

- [x] Start action from the correct player
  - Pre-flop: player after the big blind
  - Flop/Turn/River: first active player after the dealer

- [ ] Do not make only one `for` loop over the players
- [ ] Keep cycling through players until the betting round is actually complete

- [x] Detect when only one player remains
  - End the hand immediately
  - Return `false` from `betting_round()`

- [ ] Detect when everyone who can act has matched `current_bet`

- [ ] Handle raises correctly
  - If a player raises, players who acted before the raise may need to act again
  - Betting round only ends after everyone has responded to the latest bet

- [ ] Keep all-in players active in the hand
  - They can still win pots
  - They simply cannot take another action

- [ ] Detect when all remaining players are all-in
  - End the betting round
  - Remaining community cards should eventually be dealt automatically

# 2. Improve Player Decision Logic

- [ ] Implement `player_decision_set_AI()`
- [ ] Keep the first AI extremely simple
  - Check when possible
  - Call when necessary
  - Maybe fold occasionally later
  - No advanced poker strategy yet

- [ ] Print useful information before asking a human for an action
  - Player cash
  - Current contribution
  - Current bet
  - Amount required to call

- [ ] Print an error when a human selects an illegal action

# 3. Complete Raise Rules

Basic raises currently work, but proper poker rules are not fully implemented.

- [ ] Track the minimum legal raise
- [ ] Reject raises smaller than the minimum raise
- [ ] Handle minimum raise size changing after a raise

Example:

    Bet 100
    Raise to 250

The raise size was 150, so normally the next full raise must be at least another 150.

- [ ] Handle short all-in raises
- [ ] Handle the rule where a short all-in does not necessarily reopen raising

# 4. Verify All-In Logic

- [ ] Test a player calling all-in for less than `current_bet`

Example:

    current_bet = 100
    player contribution = 20
    player cash = 30

Player should contribute 30 and become all-in at 50 total.

- [ ] Test an all-in that raises the current bet
- [ ] Test several players going all-in for different amounts
- [ ] Test 3+ side pots in the same hand

# 5. Pot Edge Cases

- [ ] Test pot building with:

  contributions = {15, 30, 50, 50}

Expected:

    main pot = 60
    side pot 1 = 45
    side pot 2 = 40

- [ ] Test:

  contributions = {10, 15, 50, 50}

- [ ] Test folded players
  - Their chips remain in the pot
  - They cannot win the pot

- [ ] Handle an unmatched contribution correctly
  - If only one player contributed to the highest layer, those unmatched chips
    effectively belong back to that player rather than forming a contested pot

- [ ] Later: distribute odd chips according to poker position instead of simply
      giving them to the lowest player ID

# 6. Fix / Verify Heads-Up Rules

- [ ] Verify heads-up blind positions
  - Dealer = small blind
  - Other player = big blind

- [ ] Verify heads-up pre-flop action
  - Small blind / dealer acts first

- [ ] Verify heads-up post-flop action
  - Big blind acts first

- [ ] Verify hole-card dealing order in heads-up
  - Cards should be dealt starting with the player left of the dealer

# 7. Community Card State

- [ ] Decide whether `table->card_on_table` is actually needed

If keeping it:

- [ ] Set it to 3 after flop
- [ ] Set it to 4 after turn
- [ ] Set it to 5 after river
- [ ] Reset it to 0 after the hand

Otherwise:

- [ ] Remove the field and use the poker stage instead

# 8. Clean Up Hand Ranking

- [ ] Decide what to do with `Royal_Flush`

Currently an ace-high straight flush is correctly stronger than lower straight
flushes because of the card score, but it is still classified as
`Straight_Flush`.

Either:

- [ ] Remove `Royal_Flush` from the `rank` enum

or:

- [ ] Explicitly classify an ace-high straight flush as `Royal_Flush`

# 9. Testing

Before adding complicated AI, write tests for the game engine.

## Hand Evaluator

- [ ] High card
- [ ] Pair
- [ ] Two pair
- [ ] Three of a kind
- [ ] Straight
- [ ] 5-high straight
- [ ] Flush
- [ ] Full house
- [ ] Four of a kind
- [ ] Straight flush

- [ ] Test ties
- [ ] Test kicker comparisons
- [ ] Test `evaluate_7_cards()` selecting the correct 5 cards

## Pot Manager

- [ ] No all-ins
- [ ] One all-in
- [ ] Two different all-ins
- [ ] Three different all-ins
- [ ] Folded player contributed money
- [ ] Tied main pot
- [ ] Different winners for main pot and side pots

## Betting

- [ ] Everyone checks
- [ ] Bet -> everyone calls
- [ ] Bet -> everyone folds
- [ ] Bet -> raise -> calls
- [ ] Multiple raises
- [ ] Short all-in call
- [ ] All-in raise
- [ ] Everyone except one player folds
- [ ] Everyone remaining becomes all-in

# 10. Later Improvements

Do these only after the core game works correctly.

- [ ] Better AI
- [ ] Player names
- [ ] Better terminal UI
- [ ] Print cards nicely
- [ ] Print community cards
- [ ] Print pot sizes
- [ ] Print player stacks
- [ ] Print winners
- [ ] Hand history / logging
- [ ] Statistics
- [ ] Tournament/game-over logic
- [ ] Separate large `poker.c` into smaller modules if the file becomes annoying
      to maintain

# Next Resume Point

Start with:

    betting_round()

Do not work on advanced AI or advanced all-in raise rules yet.

First make sure a betting round can correctly:

    choose the first player
    -> process actions in order
    -> react to raises
    -> revisit players when necessary
    -> stop when all bets are matched
    -> stop the hand when only one player remains

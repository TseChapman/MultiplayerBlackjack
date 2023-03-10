#include <iostream>
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include "Deck.h"

using namespace std;

class Game {
public:
  Game(string _lobby_id, string _lobby_name);

  Game(const Game& other);

  ~Game();

  string action(string action, string player_id);

  int join(string player_id);

  int exit(string player_id);

  bool isFull();

  string getCurrentStatus();

  bool isPlayerWin(string player_id);

  string lobby_id;
  string lobby_name;
private:
  int getPlayerIndex(string player_id);

  bool isPlayerJoined(string player_id);

  string printHand(string player_id);

  int countScore(vector<string> hand);

  enum STATUS {
    READY = 0,
    START = 1,
    PLAYER_1_TURN = 2,
    PLAYER_2_TURN = 3,
    PLAYER_3_TURN = 4,
    PLAYER_4_TURN = 5,
    DEALER_TURN = 6,
    DETERMINE = 7,
    END = 8
  };

  const int MAX_PLAYERS = 4; // Number of player must joined before starting the game
  STATUS currentStatus = READY;
  STATUS nextStatus = READY;

  bool isDealerStand = false;
  vector<string> dealer_hand;
  bool isDealerLoss = false;
  vector<string> players; // player id
  unordered_map<string, STATUS> status_map;
  unordered_map<string, bool> isStand;
  unordered_map<string, vector<string>> player_hands; // num players x player hands
  unordered_map<string, bool> isLoss;
  Deck cardDeck;

  mutable mutex game_mutex;
  mutable mutex status_mutex;
};
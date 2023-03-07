#include "Game.h"

Game::Game(string _lobby_id, string _lobby_name) {
  this->lobby_id = _lobby_id;
  this->lobby_name = _lobby_name;
}

Game::Game(const Game& other) {
  lock_guard<mutex> lock(other.game_mutex);
  // Copy lobby id and name
  this->lobby_id = other.lobby_id;
  this->lobby_name = other.lobby_name;

  // Copy all the other data members
  this->currentStatus = other.currentStatus;
  this->nextStatus = other.nextStatus;
  this->isDealerStand = other.isDealerStand;
  this->dealer_hand = other.dealer_hand;
  this->players = other.players;
  this->status_map = other.status_map;
  this->isStand = other.isStand;
  this->player_hands = other.player_hands;
  this->isLoss = other.isLoss;
  this->cardDeck = other.cardDeck;
}

Game::~Game() {

}

string Game::action(string action, string player_id) {
  unique_lock<mutex> lock(game_mutex);

  if (this->currentStatus == READY) {
    return "Game have not started yet\r\n";
  }

  if (this->currentStatus == END) {
    string response = "";
    response += "Game ends, below is the scores and winners:\r\n";
    response += "Dealer's Hand:" + printHand("dealer") + " = Score: " + to_string(countScore(dealer_hand)) + ", Win:" + (this->isDealerLoss == false ? "true" : "false") + "\r\n";
    response += "Winners:\r\n";
    for (string playerId : this->players) {
      response += " |Player " + playerId + ", Hands:" + printHand(playerId) + ", Score: " + to_string(countScore(this->player_hands[playerId])) + ", Win:" + (this->isLoss[playerId] == false ? "true" : "false") + "|\r\n";
    }
    return response;
  }

  //cout << "Game::action, players.size()=" << this->players.size() << endl;
  for (string playerId : this->players) {
    //cout << "player_id=" << playerId << endl;
  }
  if (!isPlayerJoined(player_id)) {
    return "Player not joined\r\n";
  }

  // Check valid command
  unordered_map<string, int> command_map = {
    {"hit", 1},
    {"stand", 2},
    {"updates", 3},
  };

  int command;
  try {
    command = command_map[action];
  } catch (...) {
    cerr << "Command not Found" << endl;
    return "Invalid Command\r\n";
  }

  string response = "";
  switch (command) {
    case 1: {
      int player_index = getPlayerIndex(player_id);
      // Check current status
      if (this->currentStatus == this->status_map[player_id]) {
        // check if player already lost
        if (this->isStand[player_id] == false && this->isLoss[player_id] == false) {
          // Draw a card for the player
          player_hands[player_id].push_back(cardDeck.drawCard());
          // Move to next player
          if (player_index + 1 < this->players.size()) {
            this->nextStatus = this->status_map[this->players[player_index+1]];
          } else {
            // All player plays, move to dealer turn
            this->nextStatus = DEALER_TURN;
          }
          this->currentStatus = DETERMINE;
          response = action + " success\r\n";
        } else {
          // Move to next player
          if (player_index + 1 < this->players.size()) {
            this->nextStatus = this->status_map[this->players[player_index+1]];
          } else {
            // All player plays, move to dealer turn
            this->nextStatus = DEALER_TURN;
          }
          this->currentStatus = DETERMINE;
          return "player already stand\r\n";
        }

      } else {
        cout << "Not your turn" << endl;
      }

    } break;
    case 2: {
      int player_index = getPlayerIndex(player_id);
      // Check current status
      if (this->currentStatus == this->status_map[player_id]) {
        // Stand the player hand
        this->isStand[player_id] = true;
        // Move to next player
        if (player_index + 1 < this->players.size()) {
          this->nextStatus = this->status_map[this->players[player_index+1]];
          this->currentStatus = DETERMINE;
        } else {
          // All player plays, move to dealer turn
          this->nextStatus = DEALER_TURN;
          this->currentStatus = DETERMINE;
        }
      }
    } break;
    case 3: {
    } break;
  }

  // Check if it is dealer turn
  if (this->currentStatus == DEALER_TURN) {
    // Get dealer score
    int score = countScore(this->dealer_hand);
    if (score < 17 && !this->isDealerStand)
      this->dealer_hand.push_back(this->cardDeck.drawCard());
    else
      this->isDealerStand = true;

    // Determine next stage
    this->currentStatus = DETERMINE;
    int player_index = 0;
    this->nextStatus = this->status_map[this->players[player_index]];
  }

  // Determine if a winner or loser is presented
  if (this->currentStatus == DETERMINE) {
    // Get dealer score
    int dealer_score = countScore(this->dealer_hand);

    // Check if dealer score exceed 21
    if (dealer_score > 21) {
      // Dealer loss, every not lost player win
      this->isDealerLoss = true;
      this->currentStatus = END;
    }

    // Check if all player stand
    bool isAllStand = true;
    bool isAllLoss = true;
    // Get player score
    for (string playerId : this->players) {
      int player_score = countScore(this->player_hands[playerId]);

      // Check if player score exceed 21
      if (player_score > 21) {
        this->isLoss[playerId] = true;
        this->isStand[playerId] = true;
      }

      if (this->isStand[playerId] == false)
        isAllStand = false;

      if (this->isLoss[playerId] == false)
        isAllLoss = false;
    }

    if (isAllStand && this->isDealerStand) {
      // Determine winner
      for (string playerId : this->players) {
        int player_score = countScore(this->player_hands[playerId]);
        if (player_score < dealer_score) {
          // Player loss to the dealer
          this->isLoss[playerId] = true;
        }
        this->isStand[playerId] = true;
      }
      this->currentStatus = END;
    }

    if (isAllLoss) {
      this->currentStatus = END;
    }


    if (this->currentStatus != END) {
      this->currentStatus = this->nextStatus;
    }
  }

  // Return information of the game
  response += "Current turn: " + getCurrentStatus() + "\r\n";
  response += "Dealer's Hand:" + printHand("dealer") + " = Score: " + to_string(countScore(dealer_hand)) + "\r\n";
  for (string playerId : this->players) {
    response += "Player " + playerId + " Hands:" + printHand(playerId) + " = Score: " + to_string(countScore(this->player_hands[playerId])) + "\r\n";
  }

  return response;
}

int Game::join(string player_id) {
  unique_lock<mutex> lock(game_mutex);
  if (this->players.size() >= MAX_PLAYERS) {
    return 0;
  }
  if (!isPlayerJoined(player_id)) {
    this->players.push_back(player_id);
    if (this->players.size() == MAX_PLAYERS) {
      // Lobby is full, start the game
      currentStatus = PLAYER_1_TURN;
    }
    this->status_map.insert({player_id, static_cast<STATUS>(this->players.size()+1)});
    this->isStand.insert({player_id, false});
    this->isLoss.insert({player_id, false});
    //cout << "Game::join, players.size()=" << to_string(this->players.size()) << endl;
    //cout << "Game::join, isPlayerJoined=" << isPlayerJoined(player_id) << endl;
    for (string playerId : this->players) {
      //cout << "Game::join, players,playerId=" << playerId << endl;
    }
    return 1;
  }
  return 0;
}

int Game::exit(string player_id) {
  unique_lock<mutex> lock(game_mutex);
  if (isPlayerJoined(player_id)) {
    for (int i = 0; i < this->players.size(); i++) {
      if (this->players.at(i) == player_id) {
        this->players.erase(this->players.begin() + i);
        this->status_map.erase(player_id);
        this->isStand.erase(player_id);
        this->isLoss.erase(player_id);
        break;
      }
    }
    return 1; // Success on exiting player
  }
  return 0; // Fail to exit a player
}

bool Game::isFull() {
  unique_lock<mutex> lock(game_mutex);
  bool isLobbyFull = (this->players.size() >= MAX_PLAYERS);
  return isLobbyFull;
}

int Game::getPlayerIndex(string player_id) {
  int res = -1;
  for (int i = 0; i < this->players.size(); i++) {
    if (player_id == this->players[i]) {
      res = i;
      break;
    }
  }
  return res;
}

bool Game::isPlayerJoined(string player_id) {
  //cout << "isPlayerJoined: player_id='" << player_id << "'" << endl;
  //cout << "isPlayerJoined: players.size()=" << to_string(this->players.size()) << endl;
  if (this->players.empty()) {
    return false;
  }
  //cout << "isPlayerJoined: " << (find(this->players.begin(), this->players.end(), player_id) != this->players.end()) << endl;
  bool isPlayerJoinedBool = (find(this->players.begin(), this->players.end(), player_id) != this->players.end());
  return isPlayerJoinedBool;
}

string Game::printHand(string player_id) {
  string response = "";
  if (player_id == "dealer") {
    for (string card : this->dealer_hand) {
      response += card + " ";
    }
  } else {
    for (string card : this->player_hands[player_id]) {
      response += card + " ";
    }
  }
  return response;
}

string Game::getCurrentStatus() {
  unique_lock<mutex> lock(status_mutex);
  string response = "";
  if (this->currentStatus == READY) {
    response  = "READY";
  }
  else if (this->currentStatus == START) {
    response  = "START";
  }
  else if (this->currentStatus == PLAYER_1_TURN) {
    response  = "PLAYER_1_TURN";
  }
  else if (this->currentStatus == PLAYER_2_TURN) {
    response  = "PLAYER_2_TURN";
  }
  else if (this->currentStatus == PLAYER_3_TURN) {
    response  = "PLAYER_3_TURN";
  }
  else if (this->currentStatus == PLAYER_4_TURN) {
    response  = "PLAYER_4_TURN";
  }
  else if (this->currentStatus == DEALER_TURN) {
    response  = "DEALER_TURN";
  }
  else if (this->currentStatus == DETERMINE) {
    response  = "DETERMINE";
  }
  else if (this->currentStatus == END) {
    response  = "END";
  }
  return response;
}

int Game::countScore(vector<string> hand) {
  int cur = 0;
  int numAces = 0;
  for (string card : hand) {
    string value = card.substr(0,1);
    if (value == "A") {
      cur += 11;
      numAces++;
    }
    else if (value == "J" || value == "Q" || value == "K") {
      cur += 10;
    }
    else {
      int sc = stoi(value);
      cur += sc;
    }
  }

  while (cur > 21 && numAces > 0) {
    cur -= 10;
    numAces--;
  }
  return cur;
}
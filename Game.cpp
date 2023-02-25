#include "Game.h"

Game::Game(string _lobby_id, string _lobby_name) {
  lobby_id = _lobby_id;
}

int Game::join(string player_id) {
  if (players.size() >= 4) { return 0; }
  if (!isPlayerJoined(player_id)) {
    players.push_back(player_id);
    return 1;
  }
  return 0;
}

int Game::exit(string player_id) {
  if (isPlayerJoined(player_id)) {
    for (int i = 0; i < players.size(); i++) {
      if (players.at(i) == player_id) {
        players.erase(players.begin() + i);
        break;
      }
    }
    return 1; // Success on exiting player
  }
  return 0; // Fail to exit a player
}

bool Game::isFull() {
  return (players.size() < MAX_PLAYERS);
}

bool Game::isPlayerJoined(string player_id) {
  if (players.empty()) { return false; }
  return (std::find(players.begin(), players.end(), player_id) != players.end());
}
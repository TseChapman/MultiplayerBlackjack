#include <algorithm>
#include <vector>
#include <string>

using namespace std;

class Game {
public:
  Game(string _lobby_id, string _lobby_name);

  int join(string player_id);

  int exit(string player_id);

  bool isFull();

  string lobby_id;
  string lobby_name;
private:
  bool isPlayerJoined(string player_id);

  enum STATUS {
    READY = 0,
    START = 1
  };

  const int MAX_PLAYERS = 4;
  STATUS currentStatus = READY;
  vector<string> players;
};
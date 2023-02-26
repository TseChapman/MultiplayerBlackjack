#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

class Deck {
public:
  Deck();

  string drawCard();
private:
  void shuffle();

  vector<string> cards;
};
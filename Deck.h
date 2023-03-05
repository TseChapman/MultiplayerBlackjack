#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

class Deck {
public:
  Deck();

  Deck(const Deck& other);

  string drawCard();
private:
  void shuffle();

  vector<string> cards;
};
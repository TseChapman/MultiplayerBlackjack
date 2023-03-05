#include "Deck.h"

Deck::Deck() {
  // Initialize card deck
  string cardNumbers[13] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
  string cardSuits[4] = {"Clubs", "Diamonds", "Hearts", "Spades"};

  for (string suit : cardSuits) {
    for (string num : cardNumbers) {
      string card = num + "_" + suit;
      cards.push_back(card);
    }
  }
  //cout << "Cards created" << endl;
  // Shuffle the deck
  shuffle();
  //cout << "Cards Shuffled" << endl;
}

Deck::Deck(const Deck& other) {
  this->cards = other.cards;
}

string Deck::drawCard() {
  if (cards.empty()) {
    return "N/A";
  }
  // Draw the last card from the deck
  string card = cards.at(cards.size()-1);
  cards.pop_back();
  return card;
}

void Deck::shuffle() {
  // Fisher-Yates shuffle
  for (int i = 0; i < cards.size(); i++) {
    int j = rand() % cards.size();
    string temp = cards.at(i);
    cards.at(i) = cards.at(j);
    cards.at(j) = temp;
  }
}
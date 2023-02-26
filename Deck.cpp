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

  // Shuffle the deck
  shuffle();
}

string Deck::drawCard() {
  // Draw the last card from the deck
  string card = cards.at(cards.size()-1);
  cards.pop_back();
  return card;
}

void Deck::shuffle() {
  // Fisher-Yates shuffle
  for (int i = 0; i < cards.size() - 2; i++) {
    int j = rand() % cards.size() + i;
    string temp = cards.at(i);
    cards.at(i) = cards.at(j);
    cards.at(j) = temp;
  }
}
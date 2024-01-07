#include <string>
#include <iostream>
#include "Game.h"
#include "SFML/Graphics.hpp"

int main()
{
	Game game("config.txt");

	game.run();
  return 0;
}
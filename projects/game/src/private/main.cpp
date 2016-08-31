#include "Engine.h"

int main()
{
	MAD::UGameEngine gameEngine;
	if (!gameEngine.Init("A MAD Game", 1600, 900))
	{
		return 1;
	}

	gameEngine.Run();
	return 0;
}

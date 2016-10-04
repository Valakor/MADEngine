#include "Engine.h"

int main()
{
	MAD::UGameEngine gameEngine;
	if (!gameEngine.Init("A MAD Game", 2400, 1350))
	{
		return 1;
	}

	gameEngine.Run();
	return 0;
}

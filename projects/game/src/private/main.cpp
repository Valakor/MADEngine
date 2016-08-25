#include "Engine.h"

//#pragma comment(linker, "/ENTRY:mainCRTStartup")

int main()
{
	MAD::UGameEngine gameEngine;
	if (!gameEngine.Init("A MAD Game", 1600, 900))
	{
		// something
		return 1;
	}

	return gameEngine.Run();
}
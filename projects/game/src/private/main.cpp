#include "Engine.h"
#include "Core/GameWindow.h"

int main()
{
	MAD::UGameEngine gameEngine;
	eastl::shared_ptr<MAD::UGameWindow> gameWindow = eastl::make_shared<MAD::UGameWindow>();

	if (!MAD::UGameWindow::CreateGameWindow("A MAD Game", 1600, 900, *gameWindow))
	{
		return false;
	}

	if (!gameEngine.Init(gameWindow))
	{
		return 1;
	}

	gameEngine.Run();
	return 0;
}

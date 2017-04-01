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

	// ================= Game Engine Main Loop ========================
	gameEngine.ExecuteEngineTests();

	while (gameEngine.IsRunning())
	{
		gameEngine.Tick();
	}

	gameEngine.GetWindow().CaptureCursor(false);

	return 0;
}

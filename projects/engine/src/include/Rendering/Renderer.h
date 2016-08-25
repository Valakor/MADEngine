#pragma once

#include "Engine.h"

namespace MAD
{
	class URenderer
	{
	public:
		URenderer();

		bool Init();
		void Shutdown();

		void Frame();

		void OnScreenSizeChanged();

	private:
		void BeginFrame();
		void Draw();
		void EndFrame();
	};
}


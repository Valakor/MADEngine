#pragma once

namespace MAD
{
	class URenderer
	{
	public:
		URenderer();

		bool Init();
		void Shutdown();

		void Frame(float framePercent);

		void OnScreenSizeChanged();

	private:
		void BeginFrame();
		void Draw();
		void EndFrame();
	};
}

#pragma once

namespace MAD
{
	class UGameInstance
	{
	public:
		virtual ~UGameInstance() {}

		virtual void OnStartup();
		virtual void OnShutdown();
	};
}

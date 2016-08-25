#pragma once

#include "Engine.h"

namespace MAD
{
	class UGameInstance
	{
	public:
		virtual ~UGameInstance() {}
		virtual void OnStartup();
		virtual void OnShutdown();
	};

	typedef shared_ptr<class UGameInstance> UGameInstancePtr;
}

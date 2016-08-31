#pragma once

#include "EASTL/shared_ptr.h"

namespace MAD
{
	class UGameInstance
	{
	public:
		virtual ~UGameInstance() {}
		virtual void OnStartup();
		virtual void OnShutdown();
	};

	typedef eastl::shared_ptr<class UGameInstance> UGameInstancePtr;
}

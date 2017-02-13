#pragma once

#include "Core/BaseEngine.h"
#include "Networking/NetworkManager.h"

namespace MAD
{
	class UGameEngine : public UBaseEngine
	{
	public:
		virtual ~UGameEngine();
	protected:
		virtual bool Init_Internal(eastl::shared_ptr<class UGameWindow> inGameWindow) override;
		virtual void InitializeEngineContext() override;
	};
}

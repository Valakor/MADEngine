#pragma once

#include <mutex>

#include "Core/BaseEngine.h"

namespace MAD
{
	class UEditorEngine : public UBaseEngine
	{
	public:
		virtual ~UEditorEngine();

		virtual void Run() override;
	protected:
		virtual bool Init_Internal(eastl::shared_ptr<class UGameWindow> inGameWindow) override;
		virtual void InitializeEngineContext() override;
		virtual void PreTick_Internal(float inDeltaTime) override;
	private:
		std::mutex m_engineTickMutex;
	};
}

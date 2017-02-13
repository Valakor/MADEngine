#pragma once

#include "Core/BaseEngine.h"

namespace MAD
{
	class UEditorEngine : public UBaseEngine
	{
	protected:
		virtual bool Init_Internal(eastl::shared_ptr<class UGameWindow> inGameWindow) override;
		virtual void InitializeEngineContext() override;
	};
}
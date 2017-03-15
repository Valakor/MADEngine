#pragma once

#include "Core/Entity.h"

namespace MAD
{
	class ASceneCameraCharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(ASceneCameraCharacter, AEntity)
	public:
		explicit ASceneCameraCharacter(OGameWorld* inOwningWorld);
	private:
		eastl::shared_ptr<class CCameraComponent> m_cameraComponent;
	};
}
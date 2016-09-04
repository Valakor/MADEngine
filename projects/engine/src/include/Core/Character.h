#pragma once

#include "Core/Entity.h"

namespace MAD
{
	class ACharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(ACharacter, AEntity)
	public:
		ACharacter();
	};
}
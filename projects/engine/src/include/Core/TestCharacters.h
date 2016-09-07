#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

namespace MAD
{
	class AMattCharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(AMattCharacter, AEntity)
	public:
		AMattCharacter()
		{
			AddComponent<UTestComponentA>();
		}

		virtual ~AMattCharacter() {}
	};

	class ADerekCharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(ADerekCharacter, AEntity)
	public:
		ADerekCharacter()
		{
			AddComponent<UTestComponent5>();
		}

		virtual ~ADerekCharacter() {}
	};
}
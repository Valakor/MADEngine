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
			AddComponent<Test::UTestComponentA>();
		}

		virtual ~AMattCharacter() {}
	};

	class ADerekCharacter : public AEntity
	{
		MAD_DECLARE_ACTOR(ADerekCharacter, AEntity)
	public:
		ADerekCharacter()
		{
			AddComponent<Test::UTestComponent5>();
		}

		virtual ~ADerekCharacter() {}
	};
}
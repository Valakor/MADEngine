#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

namespace MAD
{
	class AMattCharacter : public ACharacter
	{
		MAD_DECLARE_ACTOR(AMattCharacter, ACharacter)
	public:
		AMattCharacter()
		{
			AddComponent<UTestComponentF>();
			AddComponent<UTestComponent2>();
			AddComponent<UTestComponentC>();
		}

		virtual ~AMattCharacter() {}
	};

	class ADerekCharacter : public ACharacter
	{
		MAD_DECLARE_ACTOR(ADerekCharacter, ACharacter)
	public:
		ADerekCharacter()
		{
			AddComponent<UTestComponent5>();
			AddComponent<UTestComponentC>();
			AddComponent<UTestComponentE>();
		}

		virtual ~ADerekCharacter() {}
	};
}
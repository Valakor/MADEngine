#pragma once

#include "Core/Character.h"
#include "Core/TestComponents.h"

namespace MAD
{
	namespace Test
	{
		class AMattCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(AMattCharacter, AEntity)
		public:
			explicit AMattCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				AddComponent<Test::CTestComponentA>();
			}
		};

		class ADerekCharacter : public AEntity
		{
			MAD_DECLARE_ACTOR(ADerekCharacter, AEntity)
		public:
			explicit ADerekCharacter(OGameWorld* inOwningWorld) : Super(inOwningWorld)
			{
				AddComponent<Test::CTestComponent5>();
			}
		};
	}
}

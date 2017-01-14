#pragma once

namespace MAD
{
	class OGameWorld;

	namespace Test
	{
		// TODO Modularized testing structure better, bare bones testing structure for now

		bool TestEntityModule(OGameWorld& inTestingGameWorld);

		bool TestEntityRootAttachment(OGameWorld& inTestingGameWorld);
	}

}
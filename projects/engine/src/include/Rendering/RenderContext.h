#pragma once

#include "Misc/Singleton.h"

namespace MAD
{
	class URenderContext : public Singleton<URenderContext>
	{
		MAD_SINGLETON(URenderContext)
	public:
		static class UGraphicsDriver& GetGraphicsDriver();
		static class URenderer& GetRenderer();
	};
}
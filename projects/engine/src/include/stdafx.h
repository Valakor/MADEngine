#pragma once

// Common platform includes
#include <SDKDDKVer.h>
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// Common library includes
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/list.h>
#include <EASTL/hash_map.h>
#include <EASTL/hash_set.h>
#include <EASTL/functional.h>
#include <EASTL/algorithm.h>
#include <EASTL/numeric.h>
#include <EASTL/numeric_limits.h>
#include <EASTL/type_traits.h>
#include <EASTL/utility.h>
#include <EASTL/memory.h>

// Common MAD Engine includes
#include "Core/Entity.h"
#include "Core/GameEngine.h"
#include "Core/GameInput.h"
#include "Core/Object.h"
#include "Core/ObjectTypeInfo.h"
#include "Core/SimpleMath.h"
#include "Misc/Assert.h"
#include "Misc/Delegate.h"
#include "Misc/Logging.h"

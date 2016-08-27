#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ENGINE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ENGINE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//#ifdef ENGINE_EXPORTS
//#define ENGINE_API __declspec(dllexport)
//#else
//#define ENGINE_API __declspec(dllimport)
//#endif

// Properly defines some members as "public to the module" vs "private to the consumer/user"
//#ifdef ENGINE_PACKAGE
//#define PACKAGE_SCOPE public
//#else
//#define PACKAGE_SCOPE protected
//#endif

// Common library includes
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>
#include <EASTL/list.h>
#include <EASTL/deque.h>
#include <EASTL/stack.h>
#include <EASTL/hash_map.h>

using eastl::string;
using eastl::vector;
using eastl::shared_ptr;
using eastl::weak_ptr;
using eastl::unique_ptr;
using eastl::array;
using eastl::list;
using eastl::deque;
using eastl::stack;
using eastl::hash_map;

// Includes!
#include "Core/GameEngine.h"
#include "Core/GameInput.h"
#include "Core/GameWindow.h"
#include "Misc/Delegate.h"
#include "Misc/Logging.h"

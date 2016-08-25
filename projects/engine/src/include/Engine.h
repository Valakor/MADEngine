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

#include <string>
#include <memory>
#include <array>
#include <list>
#include <deque>
#include <stack>
#include <unordered_map>

using std::string;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::array;
using std::list;
using std::deque;
using std::stack;
using std::unordered_map;
using std::unordered_multimap;

// Includes!
#include "Core/GameEngine.h"
#include "Core/GameInput.h"
#include "Core/GameWindow.h"
#include "Misc/Delegate.h"
#include "Misc/Logging.h"

#define IMPL_GAME_MODULE(Title) \
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, \
					  _In_opt_ HINSTANCE hPrevInstance, \
					  _In_ LPWSTR    lpCmdLine, \
					  _In_ int       nCmdShow) \
{ \
	UNREFERENCED_PARAMETER(hPrevInstance); \
	return MAD::UGameWindow::CreateGameWindow(hInstance, lpCmdLine, nCmdShow, TEXT(Title)); \
}

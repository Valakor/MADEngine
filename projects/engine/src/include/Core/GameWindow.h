#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <EASTL/string.h>

namespace MAD
{
	class UGameWindow
	{
	public:
		static bool CreateGameWindow(eastl::string inWindowTitle, int inWidth, int inHeight, UGameWindow& outGameWindow);

		static void PumpMessageQueue();

		static void ShowCursor(bool bVisible);

		static bool SetWorkingDirectory();

		UGameWindow() : hWnd(nullptr) { }

		HWND GetHWnd() const { return hWnd; }

		bool HasFocus() const;

		// Captures the cursor to this window.
		void CaptureCursor(bool bCapture) const;

		// Centers the cursor to the middle of this window.
		void CenterCursor() const;

		// Gets the position of the cursor relative to this window.
		POINT GetCursorPos() const;

		// The center of the window, where (0, 0) is the upper-left corner of the window.
		POINT GetWindowCenter() const;

	private:
		HWND hWnd;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static ATOM RegisterWindowClass(HINSTANCE inHInstance, const wchar_t* inWindowClassName);
		static HWND CreateNativeWindow(HINSTANCE inHInstance, const wchar_t* inWindowTitle, int inWidth, int inHeight);

	};
}

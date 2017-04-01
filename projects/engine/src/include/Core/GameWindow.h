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
		static bool CreateGameWindow(const eastl::string& inWindowTitle, int inWidth, int inHeight, UGameWindow& outGameWindow);
		static eastl::string GetNativeCommandline();

		static void PumpMessageQueue();

		static void ShowCursor(bool bVisible);

		static bool SetWorkingDirectory();
		static eastl::string GetWorkingDirectory();
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static ATOM RegisterWindowClass(HINSTANCE inHInstance, const wchar_t* inWindowClassName);
		static HWND CreateNativeWindow(HINSTANCE inHInstance, const wchar_t* inWindowTitle, int inWidth, int inHeight);

	public:
		explicit UGameWindow(HWND inTargetWindow = nullptr) : m_hWnd(inTargetWindow) {}

		HWND GetHWnd() const { return m_hWnd; }

		bool HasFocus() const;

		// Captures the cursor to this window.
		void CaptureCursor(bool bCapture) const;

		// Centers the cursor to the middle of this window.
		void CenterCursor() const;

		// Gets the position of the cursor relative to this window.
		POINT GetCursorPos() const;

		// The center of the window, where (0, 0) is the upper-left corner of the window.
		POINT GetWindowCenter() const;

		POINT GetClientSize() const;

		eastl::string GetWindowName() const;
		void SetWindowName(const eastl::string& inWindowName);

	private:
		static bool s_bIsResizing;
		static bool s_bWasResized;
		static bool s_bIsMinimized;

		HWND m_hWnd;
	};
}

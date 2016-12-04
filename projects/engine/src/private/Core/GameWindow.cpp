#include "Core/GameWindow.h"

#include "Core/GameEngine.h"
#include "Core/GameInput.h"
#include "Misc/Logging.h"
#include "Misc/utf8conv.h"
#include "Rendering/Renderer.h"

using eastl::string;

namespace MAD
{
	const wchar_t c_windowClassName[] = TEXT("MADEngine");
	const DWORD c_windowStyle = WS_OVERLAPPEDWINDOW;

	bool UGameWindow::s_bIsResizing = false;
	bool UGameWindow::s_bWasResized = false;
	bool UGameWindow::s_bIsMinimized = false;

	bool UGameWindow::CreateGameWindow(const string& inWindowTitle, int inWidth, int inHeight, UGameWindow& outGameWindow)
	{
		auto hInst = GetModuleHandleW(nullptr);

		if (!UGameWindow::RegisterWindowClass(hInst, c_windowClassName))
		{
			return false;
		}

		auto hWnd = UGameWindow::CreateNativeWindow(hInst, utf8util::UTF16FromUTF8(inWindowTitle).c_str(), inWidth, inHeight);
		if (!hWnd)
		{
			return false;
		}

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		outGameWindow.hWnd = hWnd;
		return true;
	}

	string UGameWindow::GetNativeCommandline()
	{
		auto wCommandLine = GetCommandLineW();
		auto exeNameEnd = wcschr(wCommandLine, ' ');
		if (exeNameEnd)
		{
			return utf8util::UTF8FromUTF16(exeNameEnd + 1);
		}
		
		return string();
	}

	void UGameWindow::PumpMessageQueue()
	{
		MSG msg = { 0 };

		// Gather any messages (probably input) since last frame
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				gEngine->Stop();
				return;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void UGameWindow::ShowCursor(bool bVisible)
	{
		static const auto defaultCursor = LoadCursorW(nullptr, IDC_ARROW);

		if (bVisible)
		{
			SetCursor(defaultCursor);
		}
		else
		{
			SetCursor(nullptr);
		}
	}

	bool UGameWindow::SetWorkingDirectory()
	{
		// Sets the engine's working directory to the directory containing the
		// launched executable.

		static wchar_t path[512];
		GetModuleFileNameW(nullptr, path, _countof(path));

		// Find the last \ and remove everything after it
		auto pos = wcsrchr(path, '\\');
		if (pos)
		{
			*pos = '\0';
		}

		return SetCurrentDirectoryW(path) != 0;
	}

	string UGameWindow::GetWorkingDirectory()
	{
		static wchar_t path[512];
		GetCurrentDirectoryW(_countof(path), path);
		return utf8util::UTF8FromUTF16(path);
	}

	ATOM UGameWindow::RegisterWindowClass(HINSTANCE inHInstance, const wchar_t* inWindowClassName)
	{
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = inHInstance;
		wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
		wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszClassName = inWindowClassName;
		wcex.lpszMenuName = nullptr;

		return RegisterClassExW(&wcex);
	}

	HWND UGameWindow::CreateNativeWindow(HINSTANCE inHInstance, const wchar_t* inWindowTitle, int inWidth, int inHeight)
	{
		// Because these are the dimensions we want for rendering, we must determine the actual
		// window size using AdjustWindowRectEx to be passed to CreateWindowEx.
		RECT windowRect = { 0, 0, inWidth, inHeight };
		AdjustWindowRect(&windowRect, c_windowStyle, FALSE);
		auto windowWidth = windowRect.right - windowRect.left;
		auto windowHeight = windowRect.bottom - windowRect.top;

		return CreateWindowExW(0, c_windowClassName, inWindowTitle, c_windowStyle,
								 CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, inHInstance, nullptr);
	}

	bool UGameWindow::HasFocus() const
	{
		return GetFocus() == hWnd;
	}

	void UGameWindow::CaptureCursor(bool bCapture) const
	{
		if (bCapture)
		{
			SetCapture(hWnd);
		}
		else
		{
			ReleaseCapture();
		}
	}

	void UGameWindow::CenterCursor() const
	{
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		auto clientWidth = clientRect.right - clientRect.left;
		auto clientHeight = clientRect.bottom - clientRect.top;

		POINT pt;
		pt.x = clientWidth / 2;
		pt.y = clientHeight / 2;
		ClientToScreen(hWnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}

	POINT UGameWindow::GetCursorPos() const
	{
		POINT pt;
		::GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		return pt;
	}

	POINT UGameWindow::GetWindowCenter() const
	{
		auto clientSize = GetClientSize();
		return{ clientSize.x / 2, clientSize.y / 2 };
	}

	POINT UGameWindow::GetClientSize() const
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		return{ rect.right - rect.left, rect.bottom - rect.top };
	}

	//
	//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
	//
	//  PURPOSE:  Processes messages for the main window.
	//
	//  WM_PAINT    - Paint the main window
	//  WM_DESTROY  - post a quit message and return
	//
	//
	LRESULT CALLBACK UGameWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		case WM_CHAR:
		{
			// For textual input (i.e. a text box)
			const auto character = static_cast<char>(wParam);
			const auto bRepeat = (lParam & (1 << 30)) != 0;

			// Send character to input text system
			UGameInput::Get().OnChar(character, bRepeat);
			break;
		}

		case WM_LBUTTONDOWN: UGameInput::Get().OnKeyDown(VK_LBUTTON, false); break;
		case WM_MBUTTONDOWN: UGameInput::Get().OnKeyDown(VK_MBUTTON, false); break;
		case WM_RBUTTONDOWN: UGameInput::Get().OnKeyDown(VK_RBUTTON, false); break;

		case WM_KEYDOWN:
		{
			// For key events (i.e. game input)
			const auto key = static_cast<UINT>(wParam);
			//const auto CharCode = MapVirtualKeyW(key, MAPVK_VK_TO_CHAR);
			const auto bRepeat = (lParam & (1 << 30)) != 0;

			// Send character to input event system
			UGameInput::Get().OnKeyDown(key, bRepeat);

			// TEMP: ESC quits the game
			if (key == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}

			// TEMP: F1 toggles exclusive fullscreen
			else if (key == VK_F1)
			{
				static bool isFullscreen = false;

				isFullscreen = !isFullscreen;
				gEngine->GetRenderer().SetFullScreen(isFullscreen);
			}

			// TEMP: F2 toggles borderless windows fullscreen
			else if (key == VK_F2)
			{
				static POINT lastRes = { 1600, 900 };
				static POINT lastPos = { 0, 0 };

				if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_POPUP)
				{
					// Leave fullscreen
					RECT targetRes = { 0, 0, lastRes.x, lastRes.y };
					AdjustWindowRect(&targetRes, c_windowStyle, FALSE);
					int w = targetRes.right - targetRes.left;
					int h = targetRes.bottom - targetRes.top;

					SetWindowLongPtrW(hWnd, GWL_STYLE, WS_VISIBLE | c_windowStyle);
					SetWindowPos(hWnd, nullptr, lastPos.x, lastPos.y, w, h, SWP_FRAMECHANGED);
				}
				else
				{
					// Enter fullscreen
					RECT currentRect;
					GetWindowRect(hWnd, &currentRect);
					lastPos.x = currentRect.left;
					lastPos.y = currentRect.top;

					GetClientRect(hWnd, &currentRect);
					lastRes.x = currentRect.right;
					lastRes.y = currentRect.bottom;

					int w = GetSystemMetrics(SM_CXSCREEN);
					int h = GetSystemMetrics(SM_CYSCREEN);
					SetWindowLongPtrW(hWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
					SetWindowPos(hWnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);
				}
			}

			break;
		}

		case WM_LBUTTONUP: UGameInput::Get().OnKeyUp(VK_LBUTTON); break;
		case WM_MBUTTONUP: UGameInput::Get().OnKeyUp(VK_MBUTTON); break;
		case WM_RBUTTONUP: UGameInput::Get().OnKeyUp(VK_RBUTTON); break;

		case WM_KEYUP:
		{
			// For key events (i.e. game input)
			//auto key = static_cast<UINT>(wParam);

			// Send character to input event system
			UGameInput::Get().OnKeyUp(wParam);
			break;
		}

		case WM_MOUSEWHEEL:
		{
			const auto wheelRotation = GET_WHEEL_DELTA_WPARAM(wParam);
			const auto numClicks = wheelRotation / WHEEL_DELTA;

			// Send numClicks to input mouse system
			(void)numClicks;
			// TODO
			break;
		}

		case WM_SIZE:
		{
			// NOTE: You NEVER want to put a breakpoint here or you'll have to restart your computer.

			s_bWasResized = true;
			if (gEngine)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					s_bIsMinimized = true;
					// TODO Suspend render loop
				}
				else if (s_bIsMinimized)
				{
					s_bIsMinimized = false;
					// TODO Resume render loop
				}
				else if (!s_bIsResizing)
				{
					gEngine->GetRenderer().OnScreenSizeChanged();
				}
			}

			break;
		}

		case WM_ENTERSIZEMOVE:
		{
			s_bIsResizing = true;
			s_bWasResized = false;
			break;
		}

		case WM_EXITSIZEMOVE:
		{
			s_bIsResizing = false;
			if (gEngine && s_bWasResized)
			{
				gEngine->GetRenderer().OnScreenSizeChanged();
			}
			break;
		}

		case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			RECT size = { 0, 0, 320, 200 };
			AdjustWindowRect(&size, c_windowStyle, FALSE);
			info->ptMinTrackSize.x = size.right - size.left;
			info->ptMinTrackSize.y = size.bottom - size.top;
			break;
		}

		case WM_ACTIVATE:
		{
			auto activeStatus = LOWORD(wParam);
			if (activeStatus == WA_ACTIVE || activeStatus == WA_CLICKACTIVE)
			{
				UGameInput::Get().OnFocusChanged(true);
			}
			else if (activeStatus == WA_INACTIVE)
			{
				UGameInput::Get().OnFocusChanged(false);
			}
			break;
		}

		default:
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}

		return 0;
	}
}

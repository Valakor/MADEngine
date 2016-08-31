#include "Core/GameWindow.h"

#include "Core/GameEngine.h"
#include "Core/GameInput.h"
#include "Misc/utf8conv.h"
#include "Rendering/Renderer.h"

using eastl::string;

namespace MAD
{
	wchar_t s_windowClassName[] = TEXT("MADEngine");

	bool UGameWindow::CreateGameWindow(const string& inWindowTitle, int inWidth, int inHeight, UGameWindow& outGameWindow)
	{
		auto hInst = GetModuleHandleW(nullptr);

		if (!UGameWindow::RegisterWindowClass(hInst, s_windowClassName))
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
		static wchar_t path[4096];
		GetModuleFileNameW(nullptr, path, _countof(path));

		// Find the last \ and remove it
		auto pos = wcsrchr(path, '\\');
		if (pos)
		{
			*pos = '\0';
		}

		return SetCurrentDirectoryW(path) != 0;
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
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		wcex.lpszClassName = inWindowClassName;
		wcex.lpszMenuName = nullptr;

		return RegisterClassExW(&wcex);
	}

	HWND UGameWindow::CreateNativeWindow(HINSTANCE inHInstance, const wchar_t* inWindowTitle, int inWidth, int inHeight)
	{
		const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
		const DWORD windowStyleEx = WS_EX_APPWINDOW;

		// Because these are the dimensions we want for rendering, we must determine the actual
		// window size using AdjustWindowRectEx to be passed to CreateWindowEx.
		RECT windowRect = { 0, 0, inWidth, inHeight };
		AdjustWindowRectEx(&windowRect, windowStyle, FALSE, windowStyleEx);
		auto windowWidth = windowRect.right - windowRect.left;
		auto windowHeight = windowRect.bottom - windowRect.top;

		return CreateWindowExW(windowStyleEx, s_windowClassName, inWindowTitle, windowStyle,
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
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		auto clientWidth = clientRect.right - clientRect.left;
		auto clientHeight = clientRect.bottom - clientRect.top;
		return{ clientWidth / 2, clientHeight / 2 };
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
			break;
		}

		case WM_KEYUP:
		{
			// For key events (i.e. game input)
			const auto key = static_cast<UINT>(wParam);

			// Send character to input event system
			UGameInput::Get().OnKeyUp(key);
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

		case WM_LBUTTONDOWN:
		{
			return 0;
		}

		case WM_MBUTTONDOWN:
		{
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			return 0;
		}

		case WM_LBUTTONUP:
		{
			return 0;
		}

		case WM_MBUTTONUP:
		{
			return 0;
		}

		case WM_RBUTTONUP:
		{
			return 0;
		}

		case WM_SIZE:
		{
			//NOTE: You NEVER want to put a breakpoint here or you'll have to restart your computer.
			if (gEngine)
			{
				gEngine->GetRenderer().OnScreenSizeChanged();
			}
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

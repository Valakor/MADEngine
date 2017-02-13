#include "EditorApplication.h"
#include "EditorMainWindow.h"

#include <Core/GameWindow.h>
#include <thread>

namespace AM
{
	EditorApplication::EditorApplication(int& argc, char** argv) : QApplication(argc, argv)
	{
	
	}

	EditorApplication::~EditorApplication()
	{
		m_nativeGameEngine.Stop();
	}

	void EditorApplication::InitApplication()
	{
		auto topLevelWidgetList = topLevelWidgets();
		EditorMainWindow* mainWindow = nullptr;

		for (auto currentWidgetIter = topLevelWidgetList.begin(); currentWidgetIter != topLevelWidgetList.end(); ++currentWidgetIter)
		{
			if (EditorMainWindow* currentWidget = qobject_cast<EditorMainWindow*>(*currentWidgetIter))
			{
				mainWindow = currentWidget;
				break;
			}
		}

		Q_ASSERT(mainWindow != nullptr);

		// Get the HWND of the panel
		HWND editorWindowHandle = reinterpret_cast<HWND>(mainWindow->GetSceneWindowId());
		eastl::shared_ptr<MAD::UGameWindow> editorWindow = eastl::make_shared<MAD::UGameWindow>(editorWindowHandle);

		if (!m_nativeGameEngine.Init(editorWindow))
		{
			return;
		}

		MAD::UGameEngine* localEngine = &m_nativeGameEngine;
		auto engineThreadLambda = [localEngine]() {localEngine->Run(); };
		std::thread engineThread(engineThreadLambda);

		engineThread.detach();
	}
}

#pragma once

#include <QEvent>
#include <Editor/EditorEngine.h>

class QEngineInterfaceEvent : public QEvent
{
public:
	QEngineInterfaceEvent(Type type = Type::User); // TODO If we ever need to identify the type of event, create custom types
	virtual ~QEngineInterfaceEvent();

	// Event execution will be guaranteed to be thread safe with the engine thread
	virtual void ExecuteInterfaceEvent(MAD::UEditorEngine& inEditorEngine) = 0;
};

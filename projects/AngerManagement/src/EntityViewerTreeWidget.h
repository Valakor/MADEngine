#pragma once

#include <QTreeWidget>

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>

#include <Core/Entity.h>

class EntityViewerTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	EntityViewerTreeWidget(QWidget* parent = Q_NULLPTR);
	~EntityViewerTreeWidget();

public slots:
	void OnEngineInitialize();
private:
	void PopulateTreeData();
private:
	eastl::vector<eastl::shared_ptr<class MAD::OGameWorld>> m_gameWorlds;
};

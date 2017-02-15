#include "EntityViewerTreeWidget.h"
#include "EditorApplication.h"

#include <Core/GameWorld.h>
#include <Core/GameWorldLayer.h>

EntityViewerTreeWidget::EntityViewerTreeWidget(QWidget* parent) : QTreeWidget(parent)
{
	
}

EntityViewerTreeWidget::~EntityViewerTreeWidget()
{
	
}

void EntityViewerTreeWidget::OnEngineInitialize()
{
	setColumnCount(1);
	
	m_gameWorlds = qEditorApp->GetEngineProxy().GetGameWorlds();

	PopulateTreeData();
}

void EntityViewerTreeWidget::PopulateTreeData()
{
	setHeaderLabel("Worlds");

	for (const auto& currentWorld : m_gameWorlds)
	{
		QTreeWidgetItem* worldTreeItem = new QTreeWidgetItem(QStringList(currentWorld->GetWorldName().c_str()));

		insertTopLevelItem(0, worldTreeItem);

		for (const auto& currentLayer : currentWorld->GetWorldLayers())
		{
			QTreeWidgetItem* worldLayerTreeItem = new QTreeWidgetItem(QStringList(currentLayer.second.GetLayerName().c_str()));

			worldTreeItem->insertChild(0, worldLayerTreeItem);

			auto currentLayerEntities = currentLayer.second.GetLayerEntities();
			const int numLayerEntities = static_cast<int>(currentLayerEntities.size());

			for (int i = 0; i < numLayerEntities; ++i)
			{
				QTreeWidgetItem* entityTreeItem = new QTreeWidgetItem(QStringList(currentLayerEntities[i]->GetDebugName().c_str()));

				worldLayerTreeItem->insertChild(i, entityTreeItem);
			}
		}
	}
	
}

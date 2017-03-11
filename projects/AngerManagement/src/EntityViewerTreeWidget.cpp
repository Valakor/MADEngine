#include "EntityViewerTreeWidget.h"
#include "EditorApplication.h"
#include "Engine/EngineCoreEvents.h"

#include <Core/Entity.h>
#include <Core/GameWorld.h>
#include <Core/GameWorldLayer.h>


EntityTreeWidgetItem::EntityTreeWidgetItem(const QStringList& inStrings, eastl::shared_ptr<MAD::AEntity> inNativeEntity)
	: QTreeWidgetItem(inStrings, EntityWidgetItemType)
	, m_nativeEntity(inNativeEntity)
{
}

EntityViewerTreeWidget::EntityViewerTreeWidget(QWidget* parent) : QTreeWidget(parent)
{
	
}

EntityViewerTreeWidget::~EntityViewerTreeWidget()
{
	
}

void EntityViewerTreeWidget::OnEngineInitialize()
{
	setColumnCount(1);
	
	m_gameWorlds = qNativeEngine.GetGameWorlds();

	PopulateTreeData();
}

void EntityViewerTreeWidget::UpdateEntityPosition(const MAD::Vector3& inNewPosition)
{
	EntityTreeWidgetItem* currentEntityItem = reinterpret_cast<EntityTreeWidgetItem*>(currentItem());

	qNativeEngine.QueueEngineEvent<QMoveEntityEvent>(currentEntityItem->GetNativeEntity(), inNewPosition);
}

void EntityViewerTreeWidget::UpdateEntityRotation(const MAD::Quaternion& inNewRotation)
{
	EntityTreeWidgetItem* currentEntityItem = reinterpret_cast<EntityTreeWidgetItem*>(currentItem());

	qNativeEngine.QueueEngineEvent<QRotateEntityEvent>(currentEntityItem->GetNativeEntity(), inNewRotation);
}

void EntityViewerTreeWidget::UpdateEntityScale(float inNewScale)
{
	EntityTreeWidgetItem* currentEntityItem = reinterpret_cast<EntityTreeWidgetItem*>(currentItem());

	qNativeEngine.QueueEngineEvent<QScaleEntityEvent>(currentEntityItem->GetNativeEntity(), inNewScale);
}

void EntityViewerTreeWidget::PopulateTreeData()
{
	setHeaderLabel("Worlds");

	for (const auto& currentWorld : m_gameWorlds)
	{
		QTreeWidgetItem* worldTreeItem = new QTreeWidgetItem(QStringList(currentWorld->GetWorldName().c_str()));

		insertTopLevelItem(0, worldTreeItem);

		worldTreeItem->setExpanded(true);

		for (const auto& currentLayer : currentWorld->GetWorldLayers())
		{
			QTreeWidgetItem* worldLayerTreeItem = new QTreeWidgetItem(QStringList(currentLayer.second.GetLayerName().c_str()));

			worldTreeItem->insertChild(0, worldLayerTreeItem);
			
			worldLayerTreeItem->setExpanded(true);

			auto currentLayerEntities = currentLayer.second.GetLayerEntities();
			const int numLayerEntities = static_cast<int>(currentLayerEntities.size());

			for (int i = 0; i < numLayerEntities; ++i)
			{
				EntityTreeWidgetItem* entityTreeItem = new EntityTreeWidgetItem(QStringList(currentLayerEntities[i]->GetDebugName().c_str()), currentLayerEntities[i]);

				worldLayerTreeItem->insertChild(i, entityTreeItem);
			}
		}
	}
	
}

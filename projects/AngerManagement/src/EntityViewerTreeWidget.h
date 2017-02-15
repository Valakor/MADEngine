#pragma once

#include <QTreeWidget>

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>

#include <Core/Entity.h>

// CAUTION: If we ever need to use signals and slots
// Limitation of moc requires that QObject needs to be first base class if the class being inherited from is not a QObject
// The auto-generated moc code will use the first parent class as a reference, which will cause compiler errors if the first
// parent class isn't inherited from QObject.
// Solution: Put QObject as first parent class before the actual inherited class
class EntityTreeWidgetItem : public QTreeWidgetItem
{
public:
	EntityTreeWidgetItem(const QStringList& inStrings, eastl::shared_ptr<class MAD::AEntity> inNativeEntity);

	eastl::shared_ptr<class MAD::AEntity> GetNativeEntity() const { return m_nativeEntity; }
private:
	eastl::shared_ptr<class MAD::AEntity> m_nativeEntity;
};

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

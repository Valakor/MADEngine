#pragma once

#include "Core/Object.h"

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

namespace MAD
{
	class UGameWorld;
	class AEntity;

	class UGameWorldLayer : public UObject
	{
		MAD_DECLARE_CLASS(UGameWorldLayer, UObject)
	public:
		virtual ~UGameWorldLayer();

		void CleanupOwnedEntities();

		inline void AddEntityToLayer(eastl::shared_ptr<AEntity> inEntity) { if (inEntity) m_layerEntities.emplace_back(inEntity); }
		inline void SetOwningWorld(UGameWorld& inGameWorld) { m_owningWorld = &inGameWorld; }
		inline void SetWorldLayerName(const eastl::string& inLayerName) { m_layerName = inLayerName; }

		inline UGameWorld& GetOwningWorld() { return *m_owningWorld; }
		inline const UGameWorld& GetOwningWorld() const { return *m_owningWorld; }
		inline const eastl::string& GetLayerName() const { return m_layerName; }
	private:
		UGameWorld* m_owningWorld;
		eastl::string m_layerName;
		eastl::vector<eastl::shared_ptr<AEntity>> m_layerEntities;
	};
}
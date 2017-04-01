#pragma once

#include "Core/Object.h"

#include <EASTL/vector.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

namespace MAD
{
	class OGameWorld;
	class AEntity;

	class OGameWorldLayer : public UObject
	{
		MAD_DECLARE_CLASS(OGameWorldLayer, UObject)
	public:
		explicit OGameWorldLayer(OGameWorld* inOwningWorld);

		virtual ~OGameWorldLayer();

		void CleanupExpiredEntities();

		inline void AddEntityToLayer(eastl::shared_ptr<AEntity> inEntity) { if (inEntity) m_layerEntities.emplace_back(inEntity); }
		inline void SetWorldLayerName(const eastl::string& inLayerName) { m_layerName = inLayerName; }

		eastl::vector<eastl::shared_ptr<AEntity>> GetLayerEntities() const { return m_layerEntities; }
		inline const eastl::string& GetLayerName() const { return m_layerName; }
		inline size_t GetEntityCount() const { return m_layerEntities.size(); }
	private:
		eastl::string m_layerName;
		eastl::vector<eastl::shared_ptr<AEntity>> m_layerEntities;
	};
}

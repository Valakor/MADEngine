#pragma once

#include "Core/Component.h"
#include "Rendering/VertexArray.h"

namespace MAD
{
	class CDebugTransformComponent : public UComponent
	{
		MAD_DECLARE_COMPONENT(CDebugTransformComponent, UComponent)
	public:
		explicit CDebugTransformComponent(OGameWorld* inOwningWorld);

		virtual void Load(const UGameWorldLoader& inLoader) override;
		virtual void UpdateComponent(float inDeltaTime) override;
	private:
		void PopulateTransformVertexArrays();
	private:
		bool m_isEnabled;
		float m_debugScale;

		UINT m_gpuVertexCount;
		eastl::vector<UVertexArray> m_gpuTransformVertexArray;
	};
}
#include "Core/DebugTransformComponent.h"
#include "Core/Pipeline/GameWorldLoader.h"
#include "Core/GameEngine.h"
#include "Rendering/Renderer.h"

namespace MAD
{
	CDebugTransformComponent::CDebugTransformComponent(OGameWorld* inOwningWorld)
		: Super_t(inOwningWorld)
		, m_isEnabled(true)
		, m_debugScale(1.0f)
	{
	}

	void CDebugTransformComponent::Load(const UGameWorldLoader& inLoader, const UObjectValue& inPropertyObj)
	{
		UNREFERENCED_PARAMETER(inLoader);

		inPropertyObj.GetProperty("enabled", m_isEnabled);
		inPropertyObj.GetProperty("debug_scale", m_debugScale);

		// Generate the vertex buffers
		PopulateTransformVertexArrays();
	}

	void CDebugTransformComponent::UpdateComponent(float)
	{
		// Submit a draw item to the debug draw
		if (!m_isEnabled)
		{
			return;
		}

		SDrawItem transformDrawItem;

		transformDrawItem.m_uniqueID = GetObjectID();
		transformDrawItem.m_vertexBuffers = m_gpuTransformVertexArray;
		transformDrawItem.m_vertexCount = m_gpuVertexCount;
		transformDrawItem.m_vertexBufferOffset = 0;

		transformDrawItem.m_indexCount = 0;
		transformDrawItem.m_indexOffset = 0;

		transformDrawItem.m_transform = GetWorldTransform();
		transformDrawItem.m_primitiveTopology = EPrimitiveTopology::LineList;
		transformDrawItem.m_drawCommand = EDrawCommand::VertexDraw;

		gEngine->GetRenderer().QueueDebugDrawItem(transformDrawItem);
	}

	void CDebugTransformComponent::PopulateTransformVertexArrays()
	{
		const uint8_t reqNumVertices = 6;
		const Vector3 unitX(m_debugScale, 0.0f, 0.0f);
		const Vector3 unitY(0.0f, m_debugScale, 0.0f);
		const Vector3 unitZ(0.0f, 0.0f, -m_debugScale);
		const Vector3 localSpaceVertexPos[] = { Vector3::Zero, unitX, Vector3::Zero, unitY, Vector3::Zero, unitZ };
		const Color transformColors[] = { Color(1.0f,0.0f,0.0f), Color(1.0f,0.0f,0.0f), Color(0.0f, 1.0f, 0.0f), Color(0.0f, 1.0f, 0.0f), Color(0.0f, 0.0f, 1.0f), Color(0.0f, 0.0f, 1.0f) }; // x(Red), y(Green), z(Blue)

		// We need 9 of each for 3 lines (each line requiring 3 Vector3s)
		eastl::array<Vector3, reqNumVertices> vertexPositionData;
		eastl::array<Vector3, reqNumVertices> vertexColorData;

		for (size_t i = 0; i < reqNumVertices; ++i)
		{
			vertexPositionData[i] = localSpaceVertexPos[i];
			vertexColorData[i] = transformColors[i].ToVector3();
		}

		m_gpuTransformVertexArray.clear();

		m_gpuVertexCount = reqNumVertices;

		// All of the positions of the transform axes
		m_gpuTransformVertexArray.emplace_back(gEngine->GetRenderer().GetGraphicsDriver(), EVertexBufferSlot::Position, EInputLayoutSemantic::Position, vertexPositionData.data(), static_cast<uint32_t>(sizeof(Vector3)), reqNumVertices);

		// All of the colors of the transform axes
		m_gpuTransformVertexArray.emplace_back(gEngine->GetRenderer().GetGraphicsDriver(), EVertexBufferSlot::Normal, EInputLayoutSemantic::Normal, vertexColorData.data(), static_cast<uint32_t>(sizeof(Vector3)), reqNumVertices);
	}

}
#pragma once

#include <EASTL/vector.h>

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/VertexDataTypes.h"

namespace MAD
{
	// STextInstance - Represents the information needed for a request to render text (each call to render text is represented as an "instance")
	struct STextInstance
	{
		eastl::string SourceText;
		float LocalCoordX;
		float LocalCoordY;
	};

	class UTextBatchRenderer
	{
	public:
		using VertexType_t = SPosTexVertex;
	public:
		void Init(const eastl::string& inFontRelativePath, uint16_t inInitialNumChars = 256);

		void BatchTextInstance(const eastl::string& inSourceString, float inScreenX, float inScreenY);

		void FlushBatch();

		bool GetTextBatchingEnabled() const { return m_isBatchRendererEnabled; }
		void SetTextBatchingEnabled(bool inIsEnabled) { m_isBatchRendererEnabled = inIsEnabled; }

		size_t GetBatchSize() const { return m_queuedTextInstances.size(); }
	private:
		void InitializeTextVertexBuffers(uint16_t inInitialNumChars);
		void BindToPipeline();
		void ProcessTextInstance(const STextInstance& inTextInstance);
		void GenerateQuadVertices(const struct SFontChar& inCharInfo, float inLocalCoordX, float inLocalCoordY);
	private:
		static const uint8_t s_numVertsPerQuad = 6;
		bool m_isBatchRendererEnabled;

		eastl::shared_ptr<class UFontFamily> m_textFontFamily;
		
		eastl::vector<STextInstance> m_queuedTextInstances;
		eastl::vector<Vector3> m_runningVBPosData;
		eastl::vector<Vector2> m_runningVBUVData;

		Matrix m_textProjectionMatrix;
		BufferPtr_t m_textBatchPosVB;
		BufferPtr_t m_textBatchTexVB;
	};
}
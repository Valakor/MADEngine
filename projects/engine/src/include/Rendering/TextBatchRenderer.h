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

		void OnScreenSizeChanged();

		void BatchTextInstance(const eastl::string& inSourceString, float inScreenX, float inScreenY);

		void FlushBatch();

		bool GetTextBatchingEnabled() const { return m_isBatchRendererEnabled; }
		void SetTextBatchingEnabled(bool inIsEnabled) { m_isBatchRendererEnabled = inIsEnabled; }

		size_t GetBatchSize() const { return m_queuedTextInstances.size(); }
	private:
		void InitializeTextVertexBuffers(uint16_t inInitialNumChars);
		void BindToPipeline();
		void UpdateCPUTextData(const STextInstance& inTextInstance);
		void GenerateQuadVertices(const struct SFontChar& inCharInfo, float inLocalCoordX, float inLocalCoordY);
	private:
		static const uint8_t s_numVertsPerQuad = 6;
		bool m_isBatchRendererEnabled;

		eastl::shared_ptr<class UFontFamily> m_textFontFamily;
		
		eastl::vector<STextInstance> m_queuedTextInstances;

		Matrix m_textProjectionMatrix;

		// Individual vertex buffer attribute vectors needed because the IA stage is setup so that each attribute has their own vertex buffer slot
		eastl::vector<Vector3> m_runningVBPosData;
		eastl::vector<Vector2> m_runningVBUVData;
		BufferPtr_t m_textBatchPosVB;
		BufferPtr_t m_textBatchTexVB;
	};
}
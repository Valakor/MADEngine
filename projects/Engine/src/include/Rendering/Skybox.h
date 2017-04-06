#pragma once

#include "Core/Component.h"
#include "Rendering/VertexArray.h"

namespace MAD
{
	class USkybox
	{
	public:
		USkybox() {}
		USkybox(const eastl::string& inShaderPath, const eastl::string& inCubemapPath, const Vector3& inBoxDimensions); // dimensions: <width, height, depth>
	
		void DrawSkybox();
	private:
		bool InitializeSkybox(const eastl::string& inShaderPath, const eastl::string& inCubemapPath);
	private:
		Vector3 m_skyboxDimensions;
		Matrix m_skyboxTransform;

		eastl::shared_ptr<class URenderPassProgram> m_skyboxShader;
		InputLayoutPtr_t m_skyboxInputLayout;

		eastl::shared_ptr<class UMesh> m_skyboxMesh;
		UVertexArray m_posGPUVB;
		
		DepthStencilPtr_t m_depthStencilView;
		DepthStencilStatePtr_t m_depthStencilState;

		BlendStatePtr_t m_skyboxBlendState;

		ShaderResourcePtr_t m_boxCubeMapSRV;
		RasterizerStatePtr_t m_boxRasterizerState;
	};
}

#include "Rendering/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Core/GameEngine.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Rendering/Renderer.h"

using Microsoft::WRL::ComPtr;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogMeshImport);

#define DO_LOG 0

#if DO_LOG == 1
#define LOG_ENABLED
#define LOG_IMPORT(Verbosity, Format, ...) LOG(LogMeshImport, Verbosity, Format, __VA_ARGS__)
#else
#define LOG_IMPORT(Verbosity, Format, ...) (void)0
#endif

	const D3D11_INPUT_ELEMENT_DESC SVertex_Pos_Tex::InputElements[NumInputElements] =
	{
		// SemanticName	SemanticIndex	Format							InputSlot	AlignedByteOffset				InputSlotClass					InstanceDataStepRate
		{ "POSITION",	0,				DXGI_FORMAT_R32G32B32_FLOAT,	0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "TEXCOORD",	0,				DXGI_FORMAT_R32G32_FLOAT,		0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
	};

	const D3D11_INPUT_ELEMENT_DESC SVertex_Pos_Norm_Tex::InputElements[NumInputElements] =
	{
		// SemanticName	SemanticIndex	Format							InputSlot	AlignedByteOffset				InputSlotClass					InstanceDataStepRate
		{ "POSITION",	0,				DXGI_FORMAT_R32G32B32_FLOAT,	0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "NORMAL",		0,				DXGI_FORMAT_R32G32B32_FLOAT,	0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "TEXCOORD",	0,				DXGI_FORMAT_R32G32_FLOAT,		0,			D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
	};

	eastl::shared_ptr<UMesh> UMesh::Load(const eastl::string& inFilePath)
	{
		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

		const aiScene* scene = importer.ReadFile(inFilePath.c_str(),
												 aiProcess_ConvertToLeftHanded |
												 aiProcessPreset_TargetRealtime_MaxQuality);

		auto path = inFilePath.substr(0, inFilePath.find_last_of('\\') + 1);

		if (!scene)
		{
			LOG_IMPORT(Error, "Failed to load mesh '%s': %s\n", inFilePath.c_str(), importer.GetErrorString());
			return nullptr;
		}

		auto mesh = eastl::make_shared<UMesh>();
		mesh->m_materials.resize(scene->mNumMaterials);
		mesh->m_subMeshes.resize(scene->mNumMeshes);

		// Process materials
		for (unsigned i = 0; i < scene->mNumMaterials; ++i)
		{
			auto aiMaterial = scene->mMaterials[i];
			auto& madMaterial = mesh->m_materials[i];
			LOG_IMPORT(Log, "Material [%d]\n", i);

			{
				aiString name;
				aiMaterial->Get(AI_MATKEY_NAME, name);
				LOG_IMPORT(Log, "\tName = %s\n", name.C_Str());
			}
			{
				int shading_model;
				aiMaterial->Get(AI_MATKEY_SHADING_MODEL, shading_model);
				LOG_IMPORT(Log, "\tShading Model = %d\n", shading_model);
			}
			{
				aiColor3D diffuse_color;
				aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
				LOG_IMPORT(Log, "\tDiffuse = { %.2f, %.2f, %.2f }\n", diffuse_color.r, diffuse_color.g, diffuse_color.b);
				madMaterial.m_mat.m_diffuseColor = DirectX::SimpleMath::Vector3(diffuse_color.r, diffuse_color.g, diffuse_color.b);

				aiString diffuse_tex;
				if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), diffuse_tex))
				{
					auto tex = UAssetCache::Load<UTexture>(path + diffuse_tex.C_Str(), false);
					if (tex)
					{
						madMaterial.m_diffuseTex = *tex;
						madMaterial.m_mat.m_bHasDiffuseTex = TRUE;
					}
				}
				LOG_IMPORT(Log, "\tDiffuse tex = %s\n", diffuse_tex.C_Str());
			}
			{
				aiColor3D specular_color;
				aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular_color);
				LOG_IMPORT(Log, "\tSpecular = { %.2f, %.2f, %.2f }\n", specular_color.r, specular_color.g, specular_color.b);
				madMaterial.m_mat.m_specularColor = DirectX::SimpleMath::Vector3(specular_color.r, specular_color.g, specular_color.b);

				float specular_power = 0.0f;
				aiMaterial->Get(AI_MATKEY_SHININESS, specular_power);
				LOG_IMPORT(Log, "\tSpecular power = %f\n", specular_power);
				madMaterial.m_mat.m_specularPower = specular_power;

				aiString specular_tex;
				if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_TEXTURE_SPECULAR(0), specular_tex))
				{
					auto tex = UAssetCache::Load<UTexture>(path + specular_tex.C_Str(), false);
					if (tex)
					{
						madMaterial.m_specularTex = *tex;
						madMaterial.m_mat.m_bHasSpecularTex = TRUE;
					}
				}
				LOG_IMPORT(Log, "\tSpecular tex = %s\n", specular_tex.C_Str());
			}
			{
				aiColor3D emissive_color;
				aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_color);
				LOG_IMPORT(Log, "\tEmissive = { %.2f, %.2f, %.2f }\n", emissive_color.r, emissive_color.g, emissive_color.b);
				madMaterial.m_mat.m_emissiveColor = DirectX::SimpleMath::Vector3(emissive_color.r, emissive_color.g, emissive_color.b);

				aiString emissive_tex;
				if (AI_SUCCESS == aiMaterial->Get(AI_MATKEY_TEXTURE_EMISSIVE(0), emissive_tex))
				{
					auto tex = UAssetCache::Load<UTexture>(path + emissive_tex.C_Str(), false);
					if (tex)
					{
						madMaterial.m_emissiveTex = *tex;
						madMaterial.m_mat.m_bHasEmissiveTex = TRUE;
					}
				}
				LOG_IMPORT(Log, "\tEmissive tex = %s\n", emissive_tex.C_Str());
			}
		}

		// Process sub-meshes
		size_t numVerts = 0;
		size_t numIndices = 0;
		for (unsigned i = 0; i < scene->mNumMeshes; ++i)
		{
			auto aiMesh = scene->mMeshes[i];

			numVerts += aiMesh->mNumVertices;
			numIndices += aiMesh->mNumFaces * 3;
		}
		mesh->m_vertexBuffer.reserve(numVerts);
		mesh->m_indexBuffer.reserve(numIndices);

		size_t currentVert = 0;
		size_t currentIndex = 0;
		for (unsigned i = 0 ; i < scene->mNumMeshes; ++i)
		{
			auto aiMesh = scene->mMeshes[i];
			auto& madSubMesh = mesh->m_subMeshes[i];
			LOG_IMPORT(Log, "Sub Mesh [%d]\n", i);

			LOG_IMPORT(Log, "\tName = %s\n", aiMesh->mName.C_Str());

			madSubMesh.m_materialIndex = aiMesh->mMaterialIndex;
			LOG_IMPORT(Log, "\tMaterial Index = %i\n", madSubMesh.m_materialIndex);

			for (unsigned v = 0; v < aiMesh->mNumVertices; ++v)
			{
				auto& pos = aiMesh->mVertices[v];
				auto& nor = aiMesh->mNormals[v];

				aiVector3D uvs;
				if (aiMesh->HasTextureCoords(0))
				{
					uvs = aiMesh->mTextureCoords[0][v];
				}
				else
				{
					uvs = aiVector3D(0);
				}

				SVertex_Pos_Norm_Tex vert;
				vert.P = DirectX::SimpleMath::Vector3(pos.x, pos.y, pos.z);
				vert.N = DirectX::SimpleMath::Vector3(nor.x, nor.y, nor.z);
				vert.T = DirectX::SimpleMath::Vector2(uvs.x, uvs.y);

				mesh->m_vertexBuffer.push_back(vert);
			}

			for (unsigned f = 0; f < aiMesh->mNumFaces; ++f)
			{
				auto& face = aiMesh->mFaces[f];
				MAD_ASSERT_DESC(face.mNumIndices == 3, "Number of indices per face must be 3");

				// Since we use a single vertex / index buffer for the entire model, we must offset
				// each sub-mesh's indices.
				Index_t i0 = static_cast<Index_t>(face.mIndices[0] + currentVert);
				Index_t i1 = static_cast<Index_t>(face.mIndices[1] + currentVert);
				Index_t i2 = static_cast<Index_t>(face.mIndices[2] + currentVert);

				mesh->m_indexBuffer.push_back(i0);
				mesh->m_indexBuffer.push_back(i1);
				mesh->m_indexBuffer.push_back(i2);
			}

			madSubMesh.m_vertexStart = currentVert;
			madSubMesh.m_vertexCount = aiMesh->mNumVertices;
			LOG_IMPORT(Log, "\tVertex Start = %i\n", madSubMesh.m_vertexStart);
			LOG_IMPORT(Log, "\tVertex Count = %i\n", madSubMesh.m_vertexCount);

			madSubMesh.m_indexStart = currentIndex;
			madSubMesh.m_indexCount = aiMesh->mNumFaces * 3;
			LOG_IMPORT(Log, "\tIndex Start = %i\n", madSubMesh.m_indexStart);
			LOG_IMPORT(Log, "\tIndex Count = %i\n", madSubMesh.m_indexCount);

			currentVert += madSubMesh.m_vertexCount;
			currentIndex += madSubMesh.m_indexCount;
		}

#ifdef LOG_ENABLED
		LOG_IMPORT(Log, "Mesh Verts:\n");
		for (unsigned i = 0; i < mesh->m_vertexBuffer.size(); ++i)
		{
			auto& v = mesh->m_vertexBuffer[i];
			LOG_IMPORT(Log, "    %2i: P { %0.2f, %0.2f, %0.2f }\n", i, v.P.x, v.P.y, v.P.z);
			LOG_IMPORT(Log, "        N { %0.2f, %0.2f, %0.2f }\n", i, v.N.x, v.N.y, v.N.z);
			LOG_IMPORT(Log, "        T { %0.2f, %0.2f }\n", i, v.T.x, v.T.y);
		}

		LOG_IMPORT(Log, "Mesh Faces:\n");
		for (unsigned i = 0; i < mesh->m_indexBuffer.size(); i += 3)
		{
			auto i0 = mesh->m_indexBuffer[i];
			auto i1 = mesh->m_indexBuffer[i + 1];
			auto i2 = mesh->m_indexBuffer[i + 2];

			LOG_IMPORT(Log, "    %2i: { %2i, %2i, %2i }\n", i / 3, i0, i1, i2);
		}
#endif

		auto& renderer = gEngine->GetRenderer();

		UINT vertexDataSize = static_cast<UINT>(mesh->m_vertexBuffer.size() * sizeof(mesh->m_vertexBuffer[0]));
		mesh->m_gpuVertexBuffer = renderer.CreateVertexBuffer(mesh->m_vertexBuffer.data(), vertexDataSize);

		UINT indexDataSize = static_cast<UINT>(mesh->m_indexBuffer.size() * sizeof(Index_t));
		mesh->m_gpuIndexBuffer = renderer.CreateIndexBuffer(mesh->m_indexBuffer.data(), indexDataSize);

		return mesh;
	}
}

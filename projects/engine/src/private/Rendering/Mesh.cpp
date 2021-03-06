#include "Rendering/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Core/GameEngine.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Rendering/GraphicsDriver.h"
#include "Rendering/Renderer.h"
#include "Rendering/DrawItem.h"
#include "Rendering/InputLayoutCache.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogMeshImport);

#define DO_LOG 0

#if defined(DO_LOG) && DO_LOG > 0
#define LOG_ENABLED
#define LOG_IMPORT(Verbosity, Format, ...) LOG(LogMeshImport, Verbosity, Format, __VA_ARGS__)
#else
#define LOG_IMPORT(Verbosity, Format, ...) (void)0
#endif

	SMeshInstance UMesh::CreatePrimitivePlane()
	{
		static bool meshLoaded = false;
		static eastl::shared_ptr<UMesh> planeMesh = eastl::make_shared<UMesh>();

		SMeshInstance retInstance;
		if (meshLoaded)
		{
			retInstance.m_mesh = planeMesh;
			return retInstance;
		}

		planeMesh->m_subMeshes.push_back_uninitialized();
		planeMesh->m_subMeshes[0].m_materialIndex = 0;
		planeMesh->m_subMeshes[0].m_indexStart = 0;
		planeMesh->m_subMeshes[0].m_indexCount = 6;
		planeMesh->m_subMeshes[0].m_vertexStart = 0;
		planeMesh->m_subMeshes[0].m_vertexCount = 4;

		auto& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();

		using namespace DirectX::SimpleMath;
		const Vector3 verts[] = { Vector3(1.0f, 1.0f, 0.0f), Vector3(-1.0f, 1.0f, 0.0f), Vector3(-1.0f, -1.0f, 0.0f), Vector3(1.0f, -1.0f, 0.0f) };
		const Index_t indices[] = { 0, 1, 2, 2, 3, 0 };
		planeMesh->m_gpuPositions = UVertexArray(graphicsDriver, EVertexBufferSlot::Position, EInputLayoutSemantic::Position, verts, sizeof(Vector3), 4);
		planeMesh->m_gpuIndexBuffer = graphicsDriver.CreateIndexBuffer(indices, 6 * sizeof(Index_t));

		retInstance.m_mesh = planeMesh;
		
		meshLoaded = true;
		return retInstance;
	}

	eastl::shared_ptr<UMesh> UMesh::Load(const eastl::string& inRelativePath)
	{
		if (auto cachedMesh = UAssetCache::GetCachedResource<UMesh>(inRelativePath))
		{
			return cachedMesh;
		}

		auto mesh = Load_Internal(inRelativePath);
		if (!mesh)
		{
			LOG(LogDefault, Warning, "Failed to load mesh: `%s`\n", inRelativePath.c_str());
			return nullptr;
		}

		LOG(LogDefault, Log, "Loaded mesh `%s`\n", inRelativePath.c_str());
		UAssetCache::InsertResource<UMesh>(inRelativePath, mesh);
		return mesh;
	}

	void UMesh::BuildDrawItems(eastl::vector<SDrawItem>& inOutTargetDrawItems, const ULinearTransform& inMeshTransform) const
	{
		const size_t subMeshCount = m_subMeshes.size();

		for (size_t i = 0; i < subMeshCount; ++i)
		{
			SDrawItem currentDrawItem;
			const UMaterial& currentMaterial = m_materials[m_subMeshes[i].m_materialIndex];
			const SGPUMaterial& currentGPUMaterial = currentMaterial.m_mat;

			// Input layout
			currentDrawItem.m_inputLayout = m_inputLayout;

			// Topology
			currentDrawItem.m_primitiveTopology = EPrimitiveTopology::TriangleList;

			// Two-sided materials will render without backface culling
			currentDrawItem.m_rasterizerState = gEngine->GetRenderer().GetRasterizerState(
				EFillMode::Solid,
				currentMaterial.m_isTwoSided ? ECullMode::None : ECullMode::Back);

			// Vertices / indices
			currentDrawItem.m_vertexBufferOffset = m_subMeshes[i].m_vertexStart;
			currentDrawItem.m_vertexCount = m_subMeshes[i].m_vertexCount;
			currentDrawItem.m_vertexBuffers.push_back(m_gpuPositions);

			if (!m_gpuNormals.Empty())
			{
				currentDrawItem.m_vertexBuffers.push_back(m_gpuNormals);
			}

			if (!m_gpuTangents.Empty() && currentMaterial.m_normalMap.GetTexureResource())
			{
				currentDrawItem.m_vertexBuffers.push_back(m_gpuTangents);
			}

			if (!m_gpuTexCoords.Empty())
			{
				currentDrawItem.m_vertexBuffers.push_back(m_gpuTexCoords);
			}

			currentDrawItem.m_indexBuffer = m_gpuIndexBuffer;
			currentDrawItem.m_indexOffset = m_subMeshes[i].m_indexStart;
			currentDrawItem.m_indexCount = m_subMeshes[i].m_indexCount;

			// Object transform
			currentDrawItem.m_transform = inMeshTransform;
			
			// Constant buffers
			currentDrawItem.m_constantBufferData.push_back({ EConstantBufferSlot::PerMaterial, { &currentGPUMaterial, static_cast<UINT>(sizeof(SGPUMaterial)) } });

			// Textures
			const ShaderResourcePtr_t& diffuseTextureResource = currentMaterial.m_diffuseTex.GetTexureResource();
			const ShaderResourcePtr_t& specularTextureResource = currentMaterial.m_specularTex.GetTexureResource();
			const ShaderResourcePtr_t& emissiveTextureResource = currentMaterial.m_emissiveTex.GetTexureResource();
			const ShaderResourcePtr_t& opacityMaskTextureResource = currentMaterial.m_opacityMask.GetTexureResource();
			const ShaderResourcePtr_t& normalMapTextureResource = currentMaterial.m_normalMap.GetTexureResource();

			if (diffuseTextureResource)
			{
				currentDrawItem.m_shaderResources.emplace_back(ETextureSlot::DiffuseMap, diffuseTextureResource);
			}

			if (specularTextureResource)
			{
				currentDrawItem.m_shaderResources.emplace_back(ETextureSlot::SpecularMap, specularTextureResource);
			}

			if (emissiveTextureResource)
			{
				currentDrawItem.m_shaderResources.emplace_back(ETextureSlot::EmissiveMap, emissiveTextureResource);
			}

			if (opacityMaskTextureResource)
			{
				currentDrawItem.m_shaderResources.emplace_back(ETextureSlot::OpacityMask, opacityMaskTextureResource);
			}

			if (normalMapTextureResource)
			{
				currentDrawItem.m_shaderResources.emplace_back(ETextureSlot::NormalMap, normalMapTextureResource);
			}

			inOutTargetDrawItems.emplace_back(currentDrawItem);
		}
	}

	eastl::shared_ptr<UMesh> UMesh::Load_Internal(const eastl::string& inRelativePath)
	{
		eastl::string fullPath = UAssetCache::GetAssetRoot() + inRelativePath;
		eastl::string path = inRelativePath.substr(0, inRelativePath.find_last_of('\\') + 1); // Everything but the mesh filename

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_LIGHTS | aiComponent_CAMERAS);
		importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
		importer.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, 80.0f);

		unsigned int flags = 0;
		flags |= aiProcess_ValidateDataStructure;
		flags |= aiProcess_RemoveRedundantMaterials;
		flags |= aiProcess_FindInstances;
		flags |= aiProcess_FindDegenerates;
		flags |= aiProcess_GenUVCoords;
		flags |= aiProcess_TransformUVCoords;
		flags |= aiProcess_Triangulate;
		flags |= aiProcess_SortByPType;
		flags |= aiProcess_FindInvalidData;
		flags |= aiProcess_OptimizeMeshes;
		//flags |= aiProcess_FixInfacingNormals;
		flags |= aiProcess_SplitLargeMeshes;
		//flags |= aiProcess_GenNormals;
		flags |= aiProcess_GenSmoothNormals;
		flags |= aiProcess_CalcTangentSpace;
		flags |= aiProcess_JoinIdenticalVertices;
		flags |= aiProcess_LimitBoneWeights;
		flags |= aiProcess_ImproveCacheLocality;
		flags |= aiProcess_FlipUVs;
		//flags |= aiProcess_FlipWindingOrder;
		//flags |= aiProcess_MakeLeftHanded;

		const aiScene* scene = importer.ReadFile(fullPath.c_str(), flags);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE)
		{
			LOG(LogMeshImport, Error, "Failed to load mesh '%s': %s\n", inRelativePath.c_str(), importer.GetErrorString());
			return nullptr;
		}

		auto mesh = eastl::make_shared<UMesh>();
		mesh->m_materials.resize(scene->mNumMaterials);
		mesh->m_subMeshes.resize(scene->mNumMeshes);

		bool hasNormalMap = false;

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
				madMaterial.m_mat.m_diffuseColor = Vector3(diffuse_color.r, diffuse_color.g, diffuse_color.b);

				aiString diffuse_tex;
				if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_tex))
				{
					auto tex = UTexture::Load(path + diffuse_tex.C_Str(), true, true);
					if (tex)
					{
						madMaterial.m_diffuseTex = *tex;
					}
				}
				LOG_IMPORT(Log, "\tDiffuse tex = %s\n", diffuse_tex.C_Str());
			}
			{
				aiColor3D specular_color;
				aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, specular_color);
				LOG_IMPORT(Log, "\tSpecular = { %.2f, %.2f, %.2f }\n", specular_color.r, specular_color.g, specular_color.b);
				madMaterial.m_mat.m_specularColor = Vector3(specular_color.r, specular_color.g, specular_color.b);

				float specular_strength = 1.0f;
				aiMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, specular_strength);
				LOG_IMPORT(Log, "\tSpecular strength = %f\n", specular_strength);
				madMaterial.m_mat.m_specularColor *= specular_strength;

				float specular_power = 1.0f;
				aiMaterial->Get(AI_MATKEY_SHININESS, specular_power);
				LOG_IMPORT(Log, "\tSpecular power = %f\n", specular_power);
				madMaterial.m_mat.m_specularPower = specular_power;

				// Assimp doesn't allow you import a reflectivity constant....using the refraction index for now
				float reflectivity = 0.0f;
				aiMaterial->Get(AI_MATKEY_REFRACTI, reflectivity);
				madMaterial.m_mat.m_reflectivity = reflectivity;

				aiString specular_tex;
				if (specular_strength > 0.0f && specular_power >= 1.0f && AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &specular_tex))
				{
					auto tex = UTexture::Load(path + specular_tex.C_Str(), true, true);
					if (tex)
					{
						madMaterial.m_specularTex = *tex;
					}
				}
				LOG_IMPORT(Log, "\tSpecular tex = %s\n", specular_tex.C_Str());
			}
			{
				aiColor3D emissive_color;
				aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_color);
				LOG_IMPORT(Log, "\tEmissive = { %.2f, %.2f, %.2f }\n", emissive_color.r, emissive_color.g, emissive_color.b);
				madMaterial.m_mat.m_emissiveColor = Vector3(emissive_color.r, emissive_color.g, emissive_color.b);

				aiString emissive_tex;
				if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &emissive_tex))
				{
					auto tex = UTexture::Load(path + emissive_tex.C_Str(), true, true);
					if (tex)
					{
						madMaterial.m_emissiveTex = *tex;
					}
				}
				LOG_IMPORT(Log, "\tEmissive tex = %s\n", emissive_tex.C_Str());
			}
			{
				aiString normal_map;
				if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &normal_map))
				{
					auto map = UTexture::Load(path + normal_map.C_Str(), false, true);
					if (map)
					{
						madMaterial.m_normalMap = *map;
						hasNormalMap = true;
					}
				}
				LOG_IMPORT(Log, "\tNormal map = %s\n", normal_map.C_Str());
			}
			{
				float opacity = 1.0f;
				aiMaterial->Get(AI_MATKEY_OPACITY, opacity);
				LOG_IMPORT(Log, "\tOpacity = %f\n", opacity);
				madMaterial.m_mat.m_opacity = opacity;

				aiString opacity_tex;
				if (AI_SUCCESS == aiMaterial->GetTexture(aiTextureType_OPACITY, 0, &opacity_tex))
				{
					auto tex = UTexture::Load(path + opacity_tex.C_Str(), true, true);
					if (tex)
					{
						madMaterial.m_opacityMask = *tex;
					}
				}
				LOG_IMPORT(Log, "\tOpacity tex = %s\n", opacity_tex.C_Str());

				int two_sided = 0;
				aiMaterial->Get(AI_MATKEY_TWOSIDED, two_sided);
				if (opacity < 1.0f || two_sided != 0 || madMaterial.m_opacityMask.GetTexureResource())
				{
					madMaterial.m_isTwoSided = true;
				}
				LOG_IMPORT(Log, "\tTwo-sided = %s\n", madMaterial.m_isTwoSided ? "true" : "false");
			}
		}

		// Process sub-meshes
		UINT numVerts = 0;
		UINT numIndices = 0;
		for (unsigned i = 0; i < scene->mNumMeshes; ++i)
		{
			auto aiMesh = scene->mMeshes[i];

			numVerts += aiMesh->mNumVertices;
			numIndices += aiMesh->mNumFaces * 3;
		}

		mesh->m_positions.reserve(numVerts);
		mesh->m_indexBuffer.reserve(numIndices);

		UINT currentVert = 0;
		UINT currentIndex = 0;
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
				auto pos = aiMesh->mVertices[v];
				mesh->m_positions.emplace_back(pos.x, pos.y, pos.z);

				if (aiMesh->HasNormals())
				{
					auto nor = aiMesh->mNormals[v];
					nor.Normalize();
					mesh->m_normals.emplace_back(nor.x, nor.y, nor.z);
				}

				if (aiMesh->HasTextureCoords(0) && aiMesh->HasNormals() && hasNormalMap && aiMesh->HasTangentsAndBitangents())
				{
					auto tangent = aiMesh->mTangents[v];
					tangent.Normalize();
					mesh->m_tangents.emplace_back(tangent.x, tangent.y, tangent.z, 1.0f);

					auto bitangent = aiMesh->mBitangents[v];
					bitangent.Normalize();

					Vector4& T = mesh->m_tangents.back();
					Vector3  B = Vector3(bitangent.x, bitangent.y, bitangent.z);
					Vector3& N = mesh->m_normals.back();

					if (Vector3(T).Cross(N).Dot(B) < 0.0f)
					{
						// Mirrored UVs
						T.w = -1.0f;
					}
				}

				if (aiMesh->HasTextureCoords(0))
				{
					auto uvs = aiMesh->mTextureCoords[0][v];
					mesh->m_texCoords.emplace_back(uvs.x, uvs.y);
				}
			}

			for (unsigned f = 0; f < aiMesh->mNumFaces; ++f)
			{
				auto& face = aiMesh->mFaces[f];
				MAD_ASSERT_DESC(face.mNumIndices == 3, "Number of indices per face must be 3");

				Index_t i0 = static_cast<Index_t>(face.mIndices[0]);
				Index_t i1 = static_cast<Index_t>(face.mIndices[1]);
				Index_t i2 = static_cast<Index_t>(face.mIndices[2]);

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

		auto& graphicsDriver = gEngine->GetRenderer().GetGraphicsDriver();
		InputLayoutFlags_t inputLayout = EInputLayoutSemantic::Position;
		const UINT vertexCount = static_cast<UINT>(mesh->m_positions.size());

		mesh->m_gpuPositions = UVertexArray(graphicsDriver, EVertexBufferSlot::Position, EInputLayoutSemantic::Position, mesh->m_positions.data(), sizeof(mesh->m_positions[0]), vertexCount);
		
		if (mesh->m_normals.size() > 0)
		{
			inputLayout |= EInputLayoutSemantic::Normal;
			mesh->m_gpuNormals = UVertexArray(graphicsDriver, EVertexBufferSlot::Normal, EInputLayoutSemantic::Normal, mesh->m_normals.data(), sizeof(mesh->m_normals[0]), vertexCount);
		}

		if (mesh->m_tangents.size() > 0)
		{
			inputLayout |= EInputLayoutSemantic::Tangent;
			mesh->m_gpuTangents = UVertexArray(graphicsDriver, EVertexBufferSlot::Tangent, EInputLayoutSemantic::Tangent, mesh->m_tangents.data(), sizeof(mesh->m_tangents[0]), vertexCount);
		}

		if (mesh->m_texCoords.size() > 0)
		{
			inputLayout |= EInputLayoutSemantic::UV;
			mesh->m_gpuTexCoords = UVertexArray(graphicsDriver, EVertexBufferSlot::UV, EInputLayoutSemantic::UV, mesh->m_texCoords.data(), sizeof(mesh->m_texCoords[0]), vertexCount);
		}

		UINT indexDataSize = static_cast<UINT>(mesh->m_indexBuffer.size() * sizeof(Index_t));
		mesh->m_gpuIndexBuffer = graphicsDriver.CreateIndexBuffer(mesh->m_indexBuffer.data(), indexDataSize);

		mesh->m_inputLayout = UInputLayoutCache::GetInputLayout(inputLayout);

		return mesh;
	}
}

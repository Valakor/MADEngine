#include "Core/GameWorldLoader.h"

#include <fstream>

#include "Engine.h"
#include "Core/Entity.h"
#include "Core/Component.h"
#include "Core/GameWorld.h"
#include "Misc/AssetCache.h"
#include "Misc/Logging.h"
#include "Rendering/Renderer.h"

using namespace rapidjson;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogGameWorldLoader);

	bool UGameWorldLoader::LoadWorld(const eastl::string& inWorldFilePath)
	{
		const eastl::string fullPath = UAssetCache::GetAssetRoot() + inWorldFilePath;

		LOG(LogGameWorldLoader, Log, "Loading game world `%s`\n", inWorldFilePath.c_str());

		std::ifstream file(fullPath.c_str());
		if (!file)
		{
			LOG(LogGameWorldLoader, Warning, "Failed to open world file `%s`\n", inWorldFilePath.c_str());
			return false;
		}

		file.seekg(0, file.end);
		size_t length = file.tellg();
		file.seekg(0, file.beg);

		eastl::string fileStr(length, '\0');
		file.read(&fileStr[0], length);

		file.close();

		StringStream jsonStr(fileStr.c_str());
		
		if (m_doc.ParseStream(jsonStr).HasParseError())
		{
			LOG(LogGameWorldLoader, Warning, "Failed to parse world file `%s`. Error: %d at %d\n", inWorldFilePath.c_str(), m_doc.GetParseError(), m_doc.GetErrorOffset());
			return false;
		}

		m_currentValue = &m_doc;

		// Load world name
		eastl::string worldName;
		if (!GetString("worldName", worldName))
		{
			LOG(LogGameWorldLoader, Warning, "No world name found. Please specify a world name\n");
			return false;
		}
		LOG(LogGameWorldLoader, Log, "World name: %s\n", worldName.c_str());

		// Create world
		eastl::weak_ptr<OGameWorld> gameWorld_weak = gEngine->SpawnGameWorld<OGameWorld>(worldName);
		eastl::shared_ptr<OGameWorld> gameWorld = gameWorld_weak.lock();

		// Load world configuration
		{
			Color ambientColor(0.2f, 0.2f, 0.2f, 1.0f);
			GetColor("ambientColor", ambientColor);
			gEngine->GetRenderer().SetWorldAmbientColor(ambientColor);

			Color backBufferClearColor(0.529f, 0.808f, 0.922f, 1.0f);
			GetColor("backBufferColor", backBufferClearColor);
			gEngine->GetRenderer().SetBackBufferClearColor(backBufferClearColor);
		}

		// Load layers
		auto layer_iter = m_doc.FindMember("layers");
		if (layer_iter == m_doc.MemberEnd() || !layer_iter->value.IsArray() || layer_iter->value.Size() < 1)
		{
			LOG(LogGameWorldLoader, Warning, "World `%s` (%s) has no layers and will be empty. Was this intentional?\n", worldName.c_str(), inWorldFilePath.c_str());
			return true;
		}
		LOG(LogGameWorldLoader, Log, "Number of layers: %i\n", layer_iter->value.Size());

		// Create and build layers one at a time
		int l_idx = 0;
		for (auto& l : layer_iter->value.GetArray())
		{
			l_idx++;
			m_currentValue = &l;

			// Read layer name
			eastl::string layerName;
			if (!GetString("name", layerName))
			{
				LOG(LogGameWorldLoader, Warning, "Layer [%i] has no name. Please specify a layer name\n", l_idx);
				continue;
			}
			LOG(LogGameWorldLoader, Log, "Layer [%i] `%s`\n", l_idx, layerName.c_str());

			// Read entities array
			auto entity_iter = l.FindMember("entities");
			if (entity_iter == l.MemberEnd() || !entity_iter->value.IsArray() || entity_iter->value.Size() < 1)
			{
				LOG(LogGameWorldLoader, Warning, "Layer [%i] `%s` has no entities. Was this intentional?\n", l_idx, layerName.c_str());
				continue;
			}

			int e_idx = 0;
			for (auto& e : entity_iter->value.GetArray())
			{
				e_idx++;
				m_currentValue = &e;

				// Read entity type name
				eastl::string entityTypeName = "AEntity";
				GetString("type", entityTypeName);

				// Get entity type info
				auto typeInfo = TTypeInfo::GetTypeInfo(entityTypeName);
				if (typeInfo == nullptr)
				{
					LOG(LogGameWorldLoader, Warning, "\tEntity [%i] `%s`: Unrecognized type name\n", e_idx, entityTypeName.c_str());
					continue;
				}

				// Spawn entity
				eastl::shared_ptr<AEntity> entity = gameWorld->SpawnEntityDeferred<AEntity>(*typeInfo, layerName);

				// Read existing components
				auto existing_iter = e.FindMember("existingComponents");
				if (existing_iter != e.MemberEnd() && existing_iter->value.IsArray())
				{
					int c_idx = 0;

					for (auto& c : existing_iter->value.GetArray())
					{
						c_idx++;
						m_currentValue = &c;

						// Read component type name
						eastl::string compTypeName;
						if (!GetString("type", compTypeName))
						{
							LOG(LogGameWorldLoader, Warning, "\t\tExisting component [%i] has no specified type name, skipping\n", c_idx);
							continue;
						}

						// Get component type info
						auto componentTypeInfo = TTypeInfo::GetTypeInfo(compTypeName);
						if (componentTypeInfo == nullptr)
						{
							LOG(LogGameWorldLoader, Warning, "\t\tExisting component [%i] `%s`: Unrecognized type name\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Find component on entity
						eastl::weak_ptr<UComponent> comp_weak = entity->GetFirstComponentByType<UComponent>(*componentTypeInfo);
						if (comp_weak.expired())
						{
							LOG(LogGameWorldLoader, Warning, "\t\tExisting component [%] `%s`: Component not found on entity\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Load the component's properties
						auto props_iter = c.FindMember("properties");
						if (props_iter == c.MemberEnd() || !props_iter->value.IsObject())
						{
							LOG(LogGameWorldLoader, Warning, "\t\tExisting component [%] `%s`: Could not load properties\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Update the component's properties
						m_currentValue = &props_iter->value;
						comp_weak.lock()->Load(*this);
					}
				}

				// Read new components
				auto new_iter = e.FindMember("newComponents");
				if (new_iter != e.MemberEnd() && new_iter->value.IsArray())
				{
					int c_idx = 0;

					for (auto& c : new_iter->value.GetArray())
					{
						c_idx++;
						m_currentValue = &c;

						// Read component type name
						eastl::string compTypeName;
						if (!GetString("type", compTypeName))
						{
							LOG(LogGameWorldLoader, Warning, "\t\tNew component [%i] has no specified type name, skipping\n", c_idx);
							continue;
						}

						// Get component type info
						auto componentTypeInfo = TTypeInfo::GetTypeInfo(compTypeName);
						if (componentTypeInfo == nullptr)
						{
							LOG(LogGameWorldLoader, Warning, "\t\tNew component [%i] `%s`: Unrecognized type name\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Add new component
						eastl::weak_ptr<UComponent> comp_weak = entity->AddComponent<UComponent>(*componentTypeInfo);
						if (comp_weak.expired())
						{
							LOG(LogGameWorldLoader, Warning, "\t\tNew component [%] `%s`: Failed to add component to entity\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Load the component's properties
						auto props_iter = c.FindMember("properties");
						if (props_iter == c.MemberEnd() || !props_iter->value.IsObject())
						{
							LOG(LogGameWorldLoader, Warning, "\t\tNew component [%] `%s`: Could not load properties\n", c_idx, compTypeName.c_str());
							continue;
						}

						// Update the component's properties
						m_currentValue = &props_iter->value;
						comp_weak.lock()->Load(*this);
					}
				}

				gameWorld->FinalizeSpawnEntity(entity);
			}
		}

		return true;
	}

	bool UGameWorldLoader::GetFloat(const char* inProp, float& outFloat) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsDouble())
		{
			return false;
		}

		outFloat = prop.GetFloat();
		return true;
	}

	bool UGameWorldLoader::GetInt(const char* inProp, int& outInt) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsInt())
		{
			return false;
		}

		outInt = prop.GetInt();
		return true;
	}

	bool UGameWorldLoader::GetString(const char* inProp, eastl::string& outString) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsString())
		{
			return false;
		}

		outString = prop.GetString();
		return true;
	}

	bool UGameWorldLoader::GetBool(const char* inProp, bool& outBool) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsBool())
		{
			return false;
		}

		outBool = prop.GetBool();
		return true;
	}

	bool UGameWorldLoader::GetVector(const char* inProp, Vector3& outVector) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsArray() || prop.Size() != 3)
		{
			return false;
		}

		for (SizeType i = 0; i < 3; i++)
		{
			if (!prop[i].IsDouble())
			{
				return false;
			}
		}

		outVector.x = prop[0].GetFloat();
		outVector.y = prop[1].GetFloat();
		outVector.z = prop[2].GetFloat();

		return true;
	}

	bool UGameWorldLoader::GetColor(const char* inProp, Color& outColor) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsArray() || prop.Size() != 4)
		{
			return false;
		}

		for (SizeType i = 0; i < 3; i++)
		{
			if (!prop[i].IsDouble())
			{
				return false;
			}
		}

		outColor.x = prop[0].GetFloat();
		outColor.y = prop[1].GetFloat();
		outColor.z = prop[2].GetFloat();
		outColor.w = prop[3].GetFloat();

		return true;
	}

	bool UGameWorldLoader::GetQuaternion(const char* inProp, Quaternion& outQuat) const
	{
		auto it = m_currentValue->FindMember(inProp);
		if (it == m_currentValue->MemberEnd())
		{
			return false;
		}

		auto& prop = it->value;
		if (!prop.IsArray() || prop.Size() != 4)
		{
			return false;
		}

		for (SizeType i = 0; i < 3; i++)
		{
			if (!prop[i].IsDouble())
			{
				return false;
			}
		}

		outQuat.x = prop[0].GetFloat();
		outQuat.y = prop[1].GetFloat();
		outQuat.z = prop[2].GetFloat();
		outQuat.w = prop[3].GetFloat();

		return true;
	}

	bool UGameWorldLoader::GetRotation(const char* inProp, Quaternion& outRot) const
	{
		Vector3 pitchYawRoll;
		if (!GetVector(inProp, pitchYawRoll))
		{
			return false;
		}

		pitchYawRoll.x = ConvertToRadians(pitchYawRoll.x);
		pitchYawRoll.y = ConvertToRadians(pitchYawRoll.y);
		pitchYawRoll.z = ConvertToRadians(pitchYawRoll.z);
		outRot = Quaternion::CreateFromYawPitchRoll(pitchYawRoll.y, pitchYawRoll.x, pitchYawRoll.z);
		return true;
	}
}

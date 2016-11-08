#include "Core/GameWorldLoader.h"

#include <fstream>
#include <rapidjson/error/en.h>

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
		m_fullFilePath = UAssetCache::GetAssetRoot() + inWorldFilePath;

		LOG(LogGameWorldLoader, Log, "Loading game world `%s`\n", inWorldFilePath.c_str());

		std::ifstream file(m_fullFilePath.c_str());
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
			LOG(LogGameWorldLoader, Warning, "Failed to parse world file `%s`. Error(offset %d) %s\n", inWorldFilePath.c_str(), m_doc.GetErrorOffset(), GetParseError_En(m_doc.GetParseError()));
			return false;
		}

		return LoadWorld(m_doc);
	}

	bool UGameWorldLoader::LoadWorld(Value& inRoot)
	{
		m_currentValue = &inRoot;

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
		m_world = gameWorld_weak.lock();

		// Load world configuration
		Color ambientColor(0.2f, 0.2f, 0.2f, 1.0f);
		GetColor("ambientColor", ambientColor);
		gEngine->GetRenderer().SetWorldAmbientColor(ambientColor);

		Color backBufferClearColor(0.529f, 0.808f, 0.922f, 1.0f);
		GetColor("backBufferColor", backBufferClearColor);
		gEngine->GetRenderer().SetBackBufferClearColor(backBufferClearColor);

		// Check layers array
		auto layer_iter = m_doc.FindMember("layers");
		if (layer_iter == m_doc.MemberEnd() || !layer_iter->value.IsArray() || layer_iter->value.Size() < 1)
		{
			LOG(LogGameWorldLoader, Warning, "World `%s` (%s) has no layers and will be empty. Was this intentional?\n", worldName.c_str(), m_fullFilePath.c_str());
			return true;
		}
		LOG(LogGameWorldLoader, Log, "Number of layers: %i\n", layer_iter->value.Size());

		// Load layers
		for (auto& layer : layer_iter->value.GetArray())
		{
			LoadLayer(layer);
		}

		return true;
	}

	bool UGameWorldLoader::LoadLayer(Value& inRoot)
	{
		m_currentValue = &inRoot;

		// Read layer name
		eastl::string layerName;
		if (!GetString("name", layerName))
		{
			LOG(LogGameWorldLoader, Warning, "Layer has no name. Please specify a layer name\n");
			return false;
		}
		LOG(LogGameWorldLoader, Log, "Layer `%s`\n", layerName.c_str());

		// Check entities array
		auto entity_iter = inRoot.FindMember("entities");
		if (entity_iter == inRoot.MemberEnd() || !entity_iter->value.IsArray() || entity_iter->value.Size() < 1)
		{
			LOG(LogGameWorldLoader, Warning, "Layer `%s` has no entities. Was this intentional?\n", layerName.c_str());
			return true;
		}

		// Load the entities
		for (auto& entity_value : entity_iter->value.GetArray())
		{
			LoadEntity(entity_value, layerName);
		}

		return true;
	}

	bool UGameWorldLoader::LoadEntity(Value& inRoot, const eastl::string& inLayerName)
	{
		m_currentValue = &inRoot;

		// Read entity type name
		eastl::string entityTypeName = "AEntity";
		GetString("type", entityTypeName);

		// Get entity type info
		auto typeInfo = TTypeInfo::GetTypeInfo(entityTypeName);
		if (typeInfo == nullptr)
		{
			LOG(LogGameWorldLoader, Warning, "\tEntity `%s`: Unrecognized type name\n", entityTypeName.c_str());
			return false;
		}

		// Spawn entity
		eastl::shared_ptr<AEntity> entity = m_world->SpawnEntityDeferred<AEntity>(*typeInfo, inLayerName);

		// Load existing components
		auto existing_iter = inRoot.FindMember("existingComponents");
		if (existing_iter != inRoot.MemberEnd() && existing_iter->value.IsArray())
		{
			for (auto& component : existing_iter->value.GetArray())
			{
				LoadExistingComponent(component, entity);
			}
		}

		// Load new components
		auto new_iter = inRoot.FindMember("newComponents");
		if (new_iter != inRoot.MemberEnd() && new_iter->value.IsArray())
		{
			for (auto& component : new_iter->value.GetArray())
			{
				LoadNewComponent(component, entity);
			}
		}

		// Finalize the deferred spawning of the entity
		m_world->FinalizeSpawnEntity(entity);

		return true;
	}

	bool UGameWorldLoader::LoadExistingComponent(Value& inRoot, eastl::shared_ptr<AEntity> inOwningEntity)
	{
		m_currentValue = &inRoot;

		// Read component type name
		eastl::string compTypeName;
		if (!GetString("type", compTypeName))
		{
			LOG(LogGameWorldLoader, Warning, "\t\tExisting component has no specified type name, skipping\n");
			return false;
		}

		// Get component type info
		auto componentTypeInfo = TTypeInfo::GetTypeInfo(compTypeName);
		if (componentTypeInfo == nullptr)
		{
			LOG(LogGameWorldLoader, Warning, "\t\tExisting component `%s`: Unrecognized type name\n", compTypeName.c_str());
			return false;
		}

		// Find component on entity
		eastl::weak_ptr<UComponent> comp_weak = inOwningEntity->GetFirstComponentByType<UComponent>(*componentTypeInfo);
		if (comp_weak.expired())
		{
			LOG(LogGameWorldLoader, Warning, "\t\tExisting component `%s`: Component type not found on entity\n", compTypeName.c_str());
			return false;
		}

		eastl::shared_ptr<UComponent> comp = comp_weak.lock();

		// Load the component's properties
		auto props_iter = inRoot.FindMember("properties");
		if (props_iter == inRoot.MemberEnd() || !props_iter->value.IsObject())
		{
			LOG(LogGameWorldLoader, Warning, "\t\tExisting component `%s`: Could not load properties\n", compTypeName.c_str());
			return false;
		}

		// Load the component's transform
		{
			Vector3 position;
			if (GetVector("position", position))
			{
				comp->SetRelativeTranslation(position);
			}

			Quaternion rotation;
			if (GetRotation("rotation", rotation))
			{
				comp->SetRelativeRotation(rotation);
			}

			float scale;
			if (GetFloat("scale", scale))
			{
				comp->SetRelativeScale(scale);
			}
		}

		// Update the component's properties
		m_currentValue = &props_iter->value;
		comp->Load(*this);

		return true;
	}

	bool UGameWorldLoader::LoadNewComponent(Value& inRoot, eastl::shared_ptr<AEntity> inOwningEntity)
	{
		m_currentValue = &inRoot;

		// Read component type name
		eastl::string compTypeName;
		if (!GetString("type", compTypeName))
		{
			LOG(LogGameWorldLoader, Warning, "\t\tNew component has no specified type name, skipping\n");
			return false;
		}

		// Get component type info
		auto componentTypeInfo = TTypeInfo::GetTypeInfo(compTypeName);
		if (componentTypeInfo == nullptr)
		{
			LOG(LogGameWorldLoader, Warning, "\t\tNew component `%s`: Unrecognized type name\n", compTypeName.c_str());
			return false;
		}

		// Load the component's properties
		auto props_iter = inRoot.FindMember("properties");
		if (props_iter == inRoot.MemberEnd() || !props_iter->value.IsObject())
		{
			LOG(LogGameWorldLoader, Warning, "\t\tNew component `%s`: Could not load properties\n", compTypeName.c_str());
			return false;
		}

		// Add new component
		eastl::shared_ptr<UComponent> comp = inOwningEntity->AddComponent<UComponent>(*componentTypeInfo);
		if (!comp)
		{
			LOG(LogGameWorldLoader, Warning, "\t\tNew component `%s`: Failed to add new component to entity\n", compTypeName.c_str());
			return false;
		}

		// Load the component's transform
		{
			Vector3 position;
			if (GetVector("position", position))
			{
				comp->SetRelativeTranslation(position);
			}

			Quaternion rotation;
			if (GetRotation("rotation", rotation))
			{
				comp->SetRelativeRotation(rotation);
			}

			float scale;
			if (GetFloat("scale", scale))
			{
				comp->SetRelativeScale(scale);
			}
		}

		// Update the component's properties
		m_currentValue = &props_iter->value;
		comp->Load(*this);

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

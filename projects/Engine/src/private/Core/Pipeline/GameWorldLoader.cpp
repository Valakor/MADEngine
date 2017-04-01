#include "Core/Pipeline/GameWorldLoader.h"

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
		m_relativeFilePath = inWorldFilePath;
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

		UObjectValue worldObjectValue(&m_doc);

		return LoadWorld(worldObjectValue);
	}

	bool UGameWorldLoader::LoadWorld(UObjectValue& inWorld)
	{
		// Load world name
		eastl::string worldName;
		if (!inWorld.GetProperty("worldName", worldName))
		{
			LOG(LogGameWorldLoader, Warning, "No world name found. Please specify a world name\n");
			return false;
		}
		LOG(LogGameWorldLoader, Log, "World name: %s\n", worldName.c_str());

		// Create world
		eastl::weak_ptr<OGameWorld> gameWorld_weak = gEngine->SpawnGameWorld<OGameWorld>(worldName, m_relativeFilePath);
		m_world = gameWorld_weak.lock();

		// Load world configuration
		Color ambientColor(0.2f, 0.2f, 0.2f, 1.0f);
		inWorld.GetProperty("ambientColor", ambientColor);
		gEngine->GetRenderer().SetWorldAmbientColor(ambientColor);

		Color backBufferClearColor(0.529f, 0.808f, 0.922f, 1.0f);
		inWorld.GetProperty("backBufferColor", backBufferClearColor);
		gEngine->GetRenderer().SetBackBufferClearColor(backBufferClearColor);

		// Check layers array
		UArrayValue layerArray;

		if (!inWorld.GetProperty("layers", layerArray))
		{
			LOG(LogGameWorldLoader, Warning, "World `%s` (%s) has no layers and will be empty. Was this intentional?\n", worldName.c_str(), m_fullFilePath.c_str());
			return true;
		}

		LOG(LogGameWorldLoader, Log, "Number of layers: %i\n", layerArray.Size());

		// Load layers
		const SizeType numLayers = layerArray.Size();

		for (SizeType i = 0; i < numLayers; ++i)
		{
			UObjectValue layerObject;

			layerArray[i].Get(layerObject);

			LoadLayer(layerObject);
		}

		return true;
	}

	bool UGameWorldLoader::LoadLayer(UObjectValue& inLayer)
	{
		// Read layer name
		eastl::string layerName;
		if (!inLayer.GetProperty("name", layerName))
		{
			LOG(LogGameWorldLoader, Warning, "Layer has no name. Please specify a layer name\n");
			return false;
		}
		LOG(LogGameWorldLoader, Log, "Layer `%s`\n", layerName.c_str());

		// Check entities array
		UArrayValue entityArray;

		if (!inLayer.GetProperty("entities", entityArray))
		{
			LOG(LogGameWorldLoader, Warning, "Layer `%s` has no entities. Was this intentional?\n", layerName.c_str());
			return true;
		}

		// Load the entities
		const SizeType numEntities = entityArray.Size();

		for (SizeType i = 0; i < numEntities; ++i)
		{
			UObjectValue entityObject;

			entityArray[i].Get(entityObject);

			LoadEntity(entityObject, layerName);
		}

		return true;
	}

	bool UGameWorldLoader::LoadEntity(UObjectValue& inEntity, const eastl::string& inLayerName)
	{
		// Read entity type name
		eastl::string entityTypeName = "AEntity";
		inEntity.GetProperty("type", entityTypeName);

		// Get entity type info
		auto typeInfo = TTypeInfo::GetTypeInfo(entityTypeName);
		if (typeInfo == nullptr)
		{
			LOG(LogGameWorldLoader, Warning, "\tEntity `%s`: Unrecognized type name\n", entityTypeName.c_str());
			return false;
		}

		// Spawn entity
		eastl::shared_ptr<AEntity> entity = m_world->SpawnEntityDeferred<AEntity>(*typeInfo, inLayerName);

		eastl::string entityDebugName = "UNASSIGNED";
		inEntity.GetProperty("name", entityDebugName);

		entity->SetDebugName(entityDebugName);

		// Load existing components
		UArrayValue existingComponentArray;
		UArrayValue newComponentArray;

		if (inEntity.GetProperty("existingComponents", existingComponentArray))
		{
			const SizeType numExistComps = existingComponentArray.Size();

			for (SizeType i = 0; i < numExistComps; ++i)
			{
				UObjectValue existingCompObj;

				existingComponentArray[i].Get(existingCompObj);

				LoadExistingComponent(existingCompObj, entity);
			}
		}

		if (inEntity.GetProperty("newComponents", newComponentArray))
		{
			const SizeType numNewComps = newComponentArray.Size();

			for (SizeType i = 0; i < numNewComps; ++i)
			{
				UObjectValue newCompObj;

				newComponentArray[i].Get(newCompObj);

				LoadNewComponent(newCompObj, entity);
			}
		}

		// Finalize the deferred spawning of the entity
		m_world->FinalizeSpawnEntity(entity);

		return true;
	}

	bool UGameWorldLoader::LoadExistingComponent(UObjectValue& inExistingComp, eastl::shared_ptr<class AEntity> inOwningEntity)
	{
		// Read component type name
		eastl::string compTypeName;
		if (!inExistingComp.GetProperty("type", compTypeName))
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
		UObjectValue compPropertyObj;

		if (!inExistingComp.GetProperty("properties", compPropertyObj))
		{
			LOG(LogGameWorldLoader, Warning, "\t\tExisting component `%s`: Could not load properties\n", compTypeName.c_str());
			return false;
		}

		// Load the component's transform
		{
			Vector3 position;
			if (inExistingComp.GetProperty("position", position))
			{
				comp->SetRelativeTranslation(position);
			}

			Vector3 rotationAngles;
			if (inExistingComp.GetProperty("rotation", rotationAngles))
			{
				rotationAngles.x = ConvertToRadians(rotationAngles.x);
				rotationAngles.y = ConvertToRadians(rotationAngles.y);
				rotationAngles.z = ConvertToRadians(rotationAngles.z);

				comp->SetRelativeRotation(FromEulerAngles(rotationAngles.x, rotationAngles.y, rotationAngles.z));
			}

			float scale;
			if (inExistingComp.GetProperty("scale", scale))
			{
				comp->SetRelativeScale(scale);
			}
		}

		// Update the component's properties
		comp->Load(*this, compPropertyObj);

		return true;
	}

	bool UGameWorldLoader::LoadNewComponent(UObjectValue& inNewComp, eastl::shared_ptr<class AEntity> inOwningEntity)
	{
		// Read component type name
		eastl::string compTypeName;
		if (!inNewComp.GetProperty("type", compTypeName))
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
		UObjectValue compPropertyObj;

		if (!inNewComp.GetProperty("properties", compPropertyObj))
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
			if (inNewComp.GetProperty("position", position))
			{
				comp->SetRelativeTranslation(position);
			}

			Vector3 rotationAngles;
			if (inNewComp.GetProperty("rotation", rotationAngles))
			{
				rotationAngles.x = ConvertToRadians(rotationAngles.x);
				rotationAngles.y = ConvertToRadians(rotationAngles.y);
				rotationAngles.z = ConvertToRadians(rotationAngles.z);

				comp->SetRelativeRotation(FromEulerAngles(rotationAngles.x, rotationAngles.y, rotationAngles.z));
			}

			float scale;
			if (inNewComp.GetProperty("scale", scale))
			{
				comp->SetRelativeScale(scale);
			}
		}

		// Update the component's properties
		comp->Load(*this, compPropertyObj);

		return true;
	}
}

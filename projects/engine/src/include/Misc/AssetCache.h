#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Misc/Assert.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogAssetCache);

	// Defines a simple interface for loading and caching resources of type T. The
	// resource must define a static function with the following declaration:
	//     static eastl::shared_ptr<T> T::Load(const eastl::string& inPath);
	class UAssetCache
	{
	public:
		UAssetCache() = delete;
		UAssetCache(const UAssetCache&) = delete;
		UAssetCache& operator=(const UAssetCache&) = delete;
		UAssetCache(UAssetCache&&) = delete;
		UAssetCache& operator=(UAssetCache&&) = delete;

		static void SetAssetRoot(const eastl::string& inPath) { s_assetRootPath = inPath; }
		static const eastl::string& GetAssetRoot() { return s_assetRootPath; }

		template <class T>
		static eastl::shared_ptr<T> Load(const eastl::string& inPath, bool inIsRelative = true);

	private:
		static eastl::string s_assetRootPath;
	};

	template <class T>
	eastl::shared_ptr<T> UAssetCache::Load(const eastl::string& inPath, bool inIsRelative)
	{
		static eastl::hash_map<eastl::string, eastl::shared_ptr<T>> s_cache;

		MAD_ASSERT_DESC(!s_assetRootPath.empty(), "Must set the asset root path before attempting to load an asset");

		auto fullPath = inIsRelative ? s_assetRootPath + inPath : inPath;
		LOG(LogAssetCache, Log, "Loading resource: %s\n", fullPath.c_str());

		auto iter = s_cache.find(fullPath);
		if (iter != s_cache.end())
		{
			return iter->second;
		}

		eastl::shared_ptr<T> loadedResource = T::Load(fullPath);

		if (!loadedResource)
		{
			LOG(LogAssetCache, Warning, "Failed to load resource: %s\n", fullPath.c_str());
			return eastl::shared_ptr<T>();
		}

		s_cache.insert({ fullPath, loadedResource });
		return eastl::shared_ptr<T>(loadedResource);
	}
}

#pragma once

#include <EASTL/hash_map.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>

#include "Misc/Assert.h"
#include "Misc/Logging.h"

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogAssetCache);

	// Defines a simple interface for loading and caching resources of type T
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

		/*
		 * Inserts a resource into a cache for resources of type T. The given path should be relative to
		 * the root of the assets directory. If a resource exists with the given path it is overwritten.
		 *
		 * Returns true if an existing resource is overwritten, false otherwise.
		 */
		template <class T>
		static bool InsertResource(const eastl::string& inResourcePath, eastl::shared_ptr<T> inResource);

		/*
		 * Retrieves a resource of type T from the given path in this cache. The given path should be relative
		 * to the root of the assets directory.
		 *
		 * Returns the cached asset if it exists, nullptr otherwise.
		 */
		template <class T>
		static eastl::shared_ptr<T> GetCachedResource(const eastl::string& inResourcePath);

	private:
		static eastl::string s_assetRootPath;

		template <class T>
		static eastl::shared_ptr<T> CacheOp(const eastl::string& inResourcePath, eastl::shared_ptr<T> inResource);
	};

	template <class T>
	bool UAssetCache::InsertResource(const eastl::string& inResourcePath, eastl::shared_ptr<T> inResource)
	{
		MAD_ASSERT_DESC(inResource != nullptr, "Cannot insert NULL resource into AssetCache");
		return CacheOp<T>(inResourcePath, inResource) != nullptr;
	}

	template <class T>
	eastl::shared_ptr<T> UAssetCache::GetCachedResource(const eastl::string& inResourcePath)
	{
		MAD_ASSERT_DESC(!s_assetRootPath.empty(), "Must set the asset root path before attempting to load an asset");
		return CacheOp<T>(inResourcePath, nullptr);
	}

	template <class T>
	eastl::shared_ptr<T> UAssetCache::CacheOp(const eastl::string& inResourcePath, eastl::shared_ptr<T> inResource)
	{
		static eastl::hash_map<eastl::string, eastl::shared_ptr<T>> s_cache;

		eastl::shared_ptr<T> ret = nullptr;

		auto iter = s_cache.find(inResourcePath);
		if (iter != s_cache.end())
		{
			ret = iter->second;
		}

		if (inResource)
		{
			// We're caching a new resource
			s_cache[inResourcePath] = inResource;
			LOG(LogAssetCache, Log, "Cached the resource: %s\n", inResourcePath.c_str());
		}

		return ret;
	}
}

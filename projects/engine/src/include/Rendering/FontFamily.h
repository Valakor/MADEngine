#pragma once

#include "Rendering/GraphicsDriverTypes.h"
#include "Rendering/Texture.h"

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#include <rapidjson/document.h>

namespace MAD
{
	struct SFontSpacing
	{
		float SpacingGap;
	};

	// Font information for a particular character
	struct SFontChar
	{
		float Width;
		float Height;
		float PosX;
		float PosY;
		SFontSpacing Spacing;
	};

	class UFontFamily
	{
	public:
		/*
		* Loads a font family at the given path. The path should be relative to the assets root directory. Internally uses a cache
		* to ensure font families are only loaded once. Will load a default font family (Arial) if the font family at the given
		* path could not be loaded. The font family will be loaded from a JSON file that includes the individual characters and their associated
		* spacing information
		*/
		static eastl::shared_ptr<UFontFamily> Load(const eastl::string& inRelativePath);

		// Given a source string, returns a vector of the corresponding character info structs
		eastl::vector<SFontChar> ConvertFromString(const eastl::string& inSourceString);

		uint32_t GetFontBitmapDimension() const { return m_fontBitmapTexture->GetWidth(); }

		ShaderResourcePtr_t GetFontTextureResource() const { return m_fontBitmapTexture->GetTexureResource(); }
	private:
		bool LoadFontFamilyJSON(const eastl::string& inRelativePath);
		bool LoadFontFamilyTexture(const eastl::string& inFullPath);
		bool LoadFontFamilyCharInfo();

		bool GetCharInfoValue(const char* inAttrName, float& inOutInfoValue) const;
	private:
		rapidjson::Document m_fontFamilyDoc;
		rapidjson::Value* m_currentValue = nullptr;

		eastl::shared_ptr<class UTexture> m_fontBitmapTexture;

		eastl::vector<SFontChar> m_fontCharInfo;
	};
}
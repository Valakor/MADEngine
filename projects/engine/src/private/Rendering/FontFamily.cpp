#include "Rendering/FontFamily.h"

#include <fstream>
#include <string>
#include <rapidjson/error/en.h>

#include "Rendering/Texture.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsDriver.h"

#include "Core/GameEngine.h"

#include "Misc/AssetCache.h"
#include "Misc/Logging.h"

using namespace rapidjson;

namespace MAD
{
	DECLARE_LOG_CATEGORY(LogFontFamily);

	eastl::shared_ptr<UFontFamily> UFontFamily::Load(const eastl::string& inRelativePath)
	{
		if (auto cachedFontFamily = UAssetCache::GetCachedResource<UFontFamily>(inRelativePath))
		{
			return cachedFontFamily;
		}

		auto resultFontFamily = eastl::make_shared<UFontFamily>();

		// Parse the JSON file to get the associated font family bitmap texture and then parse the font character information
		if (!resultFontFamily->LoadFontFamilyJSON(inRelativePath))
		{
			return nullptr;
		}

		LOG(LogFontFamily, Log, "Loaded font family `%s`. sRGB=%i generateMips=%i\n", inRelativePath.c_str(), false, false);
		UAssetCache::InsertResource<UFontFamily>(inRelativePath, resultFontFamily);

		return resultFontFamily;
	}

	bool UFontFamily::LoadFontFamilyJSON(const eastl::string& inRelativePath)
	{
		eastl::string inAbsolutePath = UAssetCache::GetAssetRoot() + inRelativePath;
		LOG(LogFontFamily, Log, "Loading font family: '%s'\n", inAbsolutePath.c_str());

		std::ifstream file(inAbsolutePath.c_str());
		if (!file)
		{
			LOG(LogFontFamily, Warning, "Failed to open font family file `%s`\n", inAbsolutePath.c_str());
			return false;
		}

		file.seekg(0, file.end);
		size_t length = file.tellg();
		file.seekg(0, file.beg);

		eastl::string fileStr(length, '\0');
		file.read(&fileStr[0], length);

		file.close();

		StringStream jsonStr(fileStr.c_str());

		if (m_fontFamilyDoc.ParseStream(jsonStr).HasParseError())
		{
			LOG(LogFontFamily, Warning, "Failed to parse font family file '%s'. Error(offset %d) %s\n", inAbsolutePath.c_str(), m_fontFamilyDoc.GetErrorOffset(), GetParseError_En(m_fontFamilyDoc.GetParseError()));
			return false;
		}

		m_currentValue = &m_fontFamilyDoc;

		// Parse the relative path to the backing bitmap texture for the font family
		eastl::string textureRelativePath;
		auto it = m_currentValue->FindMember("bitmap");

		if (it == m_currentValue->MemberEnd())
		{
			LOG(LogFontFamily, Warning, "Failed to find the path to the backing bitmap texture for the font family\n");
			return false;
		}

		auto& prop = it->value;

		if (!prop.IsString())
		{
			LOG(LogFontFamily, Warning, "The path to the backing bitmap texture for the font family is not a valid string\n");
			return false;
		}

		textureRelativePath = prop.GetString();

		// Load the font family's bitmap texture
		if (!LoadFontFamilyTexture(textureRelativePath))
		{
			LOG(LogFontFamily, Warning, "Failed to load the associated font family bitmap texture for '%s'\n", inAbsolutePath.c_str());
			return false;
		}

		// Load the font family's character information
		if (!LoadFontFamilyCharInfo())
		{
			LOG(LogFontFamily, Warning, "Failed to load the font family character information for '%s'\n", inAbsolutePath.c_str());
			return false;
		}

		return true;
	}

	bool UFontFamily::LoadFontFamilyTexture(const eastl::string& inRelativePath)
	{
		m_fontBitmapTexture = UTexture::Load(inRelativePath, false, false);

		if (!m_fontBitmapTexture)
		{
			LOG(LogFontFamily, Warning, "The backing texture for the font family bitmap couldn't be loaded\n");
			return false;
		}

		return true;
	}

	bool UFontFamily::LoadFontFamilyCharInfo()
	{
		auto charIter = m_currentValue->FindMember("char");
		auto spacingIter = m_currentValue->FindMember("spacing");

		if (charIter == m_currentValue->MemberEnd())
		{
			LOG(LogFontFamily, Warning, "The character information for the font family couldn't be found\n");
			return false;
		}

		if (spacingIter == m_currentValue->MemberEnd())
		{
			LOG(LogFontFamily, Warning, "The spacing information for the font family couldn't be found\n");
			return false;
		}

		MAD_ASSERT_DESC(charIter->value.GetArray().Size() == spacingIter->value.GetArray().Size(), "Each char information struct must have a associated spacing structure");

		const auto charArrayEnd = charIter->value.GetArray().End();
		auto charArrayIter = charIter->value.GetArray().Begin();
		auto spacingArrayIter = spacingIter->value.GetArray().Begin();

		// Iterate over the char info and the spacing info in parallel because each char has associated spacing information
		while (charArrayIter != charArrayEnd)
		{
			auto& createdCharInfo = m_fontCharInfo.push_back();

			m_currentValue = charArrayIter;

			// TODO For now we just assume they're all there because we've exported it using a tool
			GetCharInfoValue("-x", createdCharInfo.PosX);
			GetCharInfoValue("-y", createdCharInfo.PosY);
			GetCharInfoValue("-width", createdCharInfo.Width);
			GetCharInfoValue("-height", createdCharInfo.Height);

			m_currentValue = spacingArrayIter;

			// TODO If there are any other important values with spacing, parse them here
			GetCharInfoValue("-b", createdCharInfo.Spacing.SpacingGap);

			++charArrayIter;
			++spacingArrayIter;
		}

		size_t finalNumChars = m_fontCharInfo.size();

		LOG(LogFontFamily, Log, "Finished parsing the font family and detected %d characters\n", finalNumChars);

		// Reset back to the global doc value
		m_currentValue = &m_fontFamilyDoc;

		return true;
	}

	bool UFontFamily::GetCharInfoValue(const char* inAttrName, float& inOutInfoValue) const
	{
		auto it = m_currentValue->FindMember(inAttrName);

		if (it == m_currentValue->MemberEnd())
		{
			LOG(LogFontFamily, Warning, "Could get the attribute value for '%s'\n", inAttrName);
			return false;
		}

		auto& prop = it->value;

		if (!prop.IsString())
		{
			LOG(LogFontFamily, Warning, "The attribute value for '%s' is not a string\n", inAttrName);
			return false;
		}

		inOutInfoValue = std::stof(prop.GetString()); // Stupid because the exported XML file exports values as strings and not their raw numerical value

		return true;

	}

	eastl::vector<SFontChar> UFontFamily::ConvertFromString(const eastl::string& inSourceString)
	{
		eastl::vector<SFontChar> outputFontChars;

		outputFontChars.reserve(inSourceString.length());

		const uint8_t baseCharIntValue = static_cast<uint8_t>(' ');

		for (const auto& currentChar : inSourceString)
		{
			uint8_t charIntIndex = static_cast<uint8_t>(currentChar) - baseCharIntValue;
			
			outputFontChars.emplace_back(m_fontCharInfo[charIntIndex]);
		}

		return outputFontChars;
	}
}
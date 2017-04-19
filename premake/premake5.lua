require("premake-qt/qt")

local qt = premake.extensions.qt;

workspace "MAD"
	location "../projects"
	language "C++"
	architecture "x86_64"
	configurations { "DebugEditor", "DebugGame", "ReleaseEditor", "ReleaseGame" }
	flags { "FloatFast", "EnableSSE2", "StaticRuntime", "MultiProcessorCompile" }
	
	filter { "configurations:DebugEditor" }
		defines { "_DEBUG", "DEBUG", "MAD_EDITOR" }
		symbols "On"
		optimize "Off"
		inlining "Disabled"
	
	filter { "configurations:DebugGame" }
		defines { "_DEBUG", "DEBUG"}
		symbols "On"
		optimize "Off"
		inlining "Disabled"

	filter { "configurations:ReleaseEditor" }
		defines { "NDEBUG", "MAD_EDITOR" }
		optimize "Speed"
		inlining "Auto"
	
	filter { "configurations:ReleaseGame" }
		defines { "NDEBUG" }
		optimize "Speed"
		inlining "Auto"

	filter { }
	
	targetdir ("%{prj.location}/build/bin/%{cfg.longname}")
	objdir ("%{prj.location}/build/obj/%{cfg.longname}")

	function useRapidjson()
		includedirs { "../ThirdParty/rapidjson/include" }
	end

group "ThirdParty"

	project "eastl"
		location "../projects/ThirdParty/eastl"
		kind "StaticLib"
		files "../projects/ThirdParty/eastl/src/**"
		includedirs "../projects/ThirdParty/eastl/src/include"
		rtti "Off"
		defines { "_CHAR16T", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS", "EASTL_OPENSOURCE=1", "EA_COMPILER_NO_RTTI", "EA_COMPILER_NO_EXCEPTIONS" }

	function useEastl()
		includedirs "../projects/ThirdParty/eastl/src/include"
		links "eastl"
		defines { "NOMINMAX" }
	end

	project "yojimbo"
		location "../projects/ThirdParty/yojimbo"
		kind "StaticLib"
		files "../projects/ThirdParty/yojimbo/src/**"
		includedirs "../projects/ThirdParty/yojimbo/src/include/yojimbo"
		rtti "Off"
		useRapidjson()

		if os.is "windows" then
			includedirs "../projects/ThirdParty/yojimbo/windows"
		else
			includedirs "/usr/local/include"  -- for clang scan-build only. for some reason it needs this to work =p
		end

	function useYojimbo()
		includedirs "../projects/ThirdParty/yojimbo/src/include"

		libdirs { "../projects/ThirdParty/yojimbo/windows" }
		if os.is "windows" then
			debug_libs = { "sodium-debug", "mbedtls-debug", "mbedx509-debug", "mbedcrypto-debug" }
			release_libs = { "sodium-release", "mbedtls-release", "mbedx509-release", "mbedcrypto-release" }
		else
			debug_libs = { "sodium", "mbedtls", "mbedx509", "mbedcrypto" }
			release_libs = debug_libs
		end

		filter { "configurations:DebugEditor or DebugGame" }
			links { "yojimbo", debug_libs }
		filter { "configurations:ReleaseEditor or ReleaseGame" }
			links { "yojimbo", release_libs }

		filter { }
	end

group ""

function useAssimp()
	libdirs { "../ThirdParty/assimp/lib" }
	includedirs { "../ThirdParty/assimp/include" }

	filter { "configurations:DebugEditor or DebugGame" }
		links { "zlibstaticD", "assimpD" }

	filter { "configurations:ReleaseEditor or ReleaseGame" }
		links { "zlibstatic", "assimp" }

	filter { }
end

function useDirectX()
	links { "d3d11", "dxgi", "d3dcompiler" }
end

function useDirectXTK()
	libdirs { "../ThirdParty/DirectXTK/lib" }
	includedirs { "../ThirdParty/DirectXTK/include" }

	filter { "configurations:DebugEditor or DebugGame" }
		links { "DirectXTKD" }

	filter { "configurations:ReleaseEditor or ReleaseGame" }
		links { "DirectXTK" }

	filter { }
end

function useQT()
	qt.enable();
	qtpath "$(QTDIR)"
	qtgenerateddir "../projects/AngerManagement/GeneratedFiles"
	qtprefix "Qt5"
	qtmodules { "core", "gui", "widgets" }
	filter { "configurations:DebugEditor or DebugGame"}
		qtsuffix "d"
	filter {}
end

function commonSetup()
	rtti "Off"
	warnings "Extra"
	flags { "FatalWarnings" }

	useEastl()
	useAssimp()
	useDirectX()
	useDirectXTK()
	useRapidjson()
	useYojimbo()
end

project "Engine"
	location "../projects/Engine"
	kind "StaticLib"
	files { "../projects/Engine/src/**", "../projects/Assets/engine/shaders/**", "../projects/Assets/engine/worlds/**" }
	includedirs { "../projects/Engine/src/include" }
	pchheader "stdafx.h"
	pchsource "../projects/Engine/src/private/stdafx.cpp"
	forceincludes { "stdafx.h" }
	commonSetup()

function useEngine()
	includedirs "../projects/Engine/src/include"
	links "Engine"
end

project "AngerManagement"
	location "../projects/AngerManagement"
	kind "WindowedApp"
	files "../projects/AngerManagement/src/**"
	useQT()
	includedirs { "../projects/AngerManagement/src" } -- self inclusion so that the auto-gen Qt files can refer to custom widget headers
	useEngine()
	-- Figure if there is a way of specifying the AngerManagement project to inherit include directories from the Engine project (?)
	commonSetup()

	postbuildcommands { "call \"$(SolutionDir)..\\premake\\MADStage.bat\" \"%{prj.name}\" \"$(TargetDir)\" \"$(SolutionDir)\"" }

project "Game"
	location "../projects/Game"
	kind "WindowedApp"
	files "../projects/Game/src/**"
	commonSetup()
	useEngine()
	entrypoint "mainCRTStartup"

	postbuildcommands { "call \"$(SolutionDir)..\\premake\\MADStage.bat\" \"%{prj.name}\" \"$(TargetDir)\" \"$(SolutionDir)\"" }

workspace "MAD"
	location "../projects"
	language "C++"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	flags { "FloatFast", "EnableSSE2", "StaticRuntime", "MultiProcessorCompile" }
	
	filter { "configurations:Debug" }
		defines { "_DEBUG", "DEBUG" }
		symbols "On"
		optimize "Off"
		inlining "Disabled"
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		symbols "Off"
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

		filter { "configurations:Debug" }
			links { "yojimbo", debug_libs }
		filter { "configurations:Release" }
			links { "yojimbo", release_libs }

		filter { }
	end

group ""

function useAssimp()
	libdirs { "../ThirdParty/assimp/lib" }
	includedirs { "../ThirdParty/assimp/include" }

	filter { "configurations:Debug" }
		links { "zlibstaticD", "assimpD" }

	filter { "configurations:Release" }
		links { "zlibstatic", "assimp" }

	filter { }
end

function useDirectX()
	links { "d3d11", "dxgi", "d3dcompiler" }
end

function useDirectXTK()
	libdirs { "../ThirdParty/DirectXTK/lib" }
	includedirs { "../ThirdParty/DirectXTK/include" }

	filter { "configurations:Debug" }
		links { "DirectXTKD" }

	filter { "configurations:Release" }
		links { "DirectXTK" }

	filter { }
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

project "engine"
	location "../projects/engine"
	kind "StaticLib"
	files "../projects/engine/src/**"
	includedirs { "../projects/engine/src/include" }
	pchheader "stdafx.h"
	pchsource "../projects/engine/src/private/stdafx.cpp"
	forceincludes { "stdafx.h" }
	commonSetup()

function useEngine()
	includedirs "../projects/engine/src/include"
	links "engine"
end

project "game"
	location "../projects/game"
	kind "WindowedApp"
	files "../projects/game/src/**"
	commonSetup()
	useEngine()
	entrypoint "mainCRTStartup"

	postbuildcommands { "if not exist $(TargetDir)assets mklink /J $(TargetDir)assets $(SolutionDir)assets" }

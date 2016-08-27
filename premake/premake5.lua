workspace "MAD"
	location "../projects"
	language "C++"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	
	filter { "configurations:Debug" }
		defines { "_DEBUG" }
		flags { "Symbols" }
		optimize "Off"
		inlining "Disabled"
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "Speed"
		inlining "Auto"
	
	filter { }
	
	targetdir ("../projects/%{prj.name}/build/bin/%{cfg.longname}")
	objdir ("../projects/%{prj.name}/build/obj/%{cfg.longname}")

group "ThirdParty"

	project "EASTL"
		location "../projects/ThirdParty/eastl"
		kind "StaticLib"
		files "../projects/ThirdParty/eastl/src/**"
		includedirs "../projects/ThirdParty/eastl/src/include"
		rtti "Off"
		defines { "_CHAR16T", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS", "EASTL_OPENSOURCE=1" }

	function useEastl()
		includedirs "../projects/ThirdParty/eastl/src/include"
		links "EASTL"
	end

group ""

function commonSetup()
	rtti "Off"
	warnings "Extra"
	flags { "FatalWarnings" }
end

project "engine"
	location "../projects/engine"
	kind "StaticLib"
	files "../projects/engine/src/**"
	includedirs { "../projects/engine/src/include", "../projects/engine/src/include_private" }
	commonSetup()
	useEastl()

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
	useEastl()
	entrypoint "mainCRTStartup"

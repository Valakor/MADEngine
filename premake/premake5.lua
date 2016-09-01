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
	
	targetdir ("%{prj.location}/build/bin/%{cfg.longname}")
	objdir ("%{prj.location}/build/obj/%{cfg.longname}")

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

group ""

function useAssimp()
	libdirs { "../ThirdParty/assimp/lib" }
	includedirs { "../ThirdParty/assimp/include" }

	filter { "configurations:Debug" }
		links { "zlibstaticD", "assimp-vc140-mtD" }

	filter { "configurations:Release" }
		links { "zlibstatic", "assimp-vc140-mt" }

	filter { }
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
	useDirectXTK()
end

project "engine"
	location "../projects/engine"
	kind "StaticLib"
	files "../projects/engine/src/**"
	includedirs { "../projects/engine/src/include" }
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

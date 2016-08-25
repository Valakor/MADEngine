workspace "MAD"
	location "../projects"
	language "C++"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	
	filter { "configurations:Debug" }
		defines { "DEBUG" }
		flags { "Symbols" }
	
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"
	
	filter { }
	
	targetdir ("../projects/%{prj.name}/build/bin/%{cfg.longname}")
	objdir ("../projects/%{prj.name}/build/obj/%{cfg.longname}")

project "engine"
	location "../projects/engine"
	kind "StaticLib"
	files "../projects/engine/src/**"

function useEngine()
	includedirs "../projects/engine/src/include"
	links "engine"
end

project "game"
	location "../projects/game"
	kind "WindowedApp"
	files "../projects/game/src/**"
	useEngine()
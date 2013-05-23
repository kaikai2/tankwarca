project "tankwar"
	kind "WindowedApp"
	location ("../build/" .. _ACTION)
	targetname "tankwar"
	targetdir "../bin"
	targetextension ".exe"
	-- links {"curvedani", "glport"}
	files
	{
		"../include/**.h",
		"../src/**.cpp",
		"../src/**.h"
	}
	excludes
	{
		"fsm/**.*",
	}
	includedirs
	{
		"../include",
		"../include/ca",
		"../include/hge",
	}
	libdirs
	{
		"../lib",
		"../lib/" .. _ACTION,
	}
	flags
	{
		"NoPCH",
		"StaticRuntime",
		"WinMain",
	}
	if (target == "vs2005") then
		defines = { "_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE" }
	end
	defines { "_WINDOWS", "USE_PROFILER=1" }
	configuration "Debug"
	defines { "_DEBUG" }
	links
	{
		"curvedani_d",
		"hgeport_d",
		"hge",
		"hgehelp",
	}
	flags
	{
		"Symbols", 
	}
	configuration "Release"
	defines { "NDEBUG" }
	links
	{

		"curvedani",
		"hgeport",
		"hge",
		"hgehelp",

	}
	flags
	{
		"OptimizeSpeed",
	}

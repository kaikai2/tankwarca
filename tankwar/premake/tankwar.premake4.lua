solution "TankWar"
	configurations { "Debug", "Release" }
	language "C++"
	--  platforms({ "Native" })
	location ( "../build/" .. _ACTION)
	defines { "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS" }

dofile  "tankwar_project_premake4.lua"

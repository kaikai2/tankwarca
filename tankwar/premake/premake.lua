-- target 是预定义变量,表示编译环境的名字
project.path = "../build/" .. target
project.name = "tankwar"
project.bindir = "../bin"
project.libdir = "../lib"


-----------------------------
-- Set the Active Project
-- Node: first package will be active
-----------------------------
packagePathFinding = newpackage()
packageTankWar = newpackage()

-----------------------------
-- TankWar package
-----------------------------

package = packageTankWar
package.path = "../build/" .. target
package.kind = "winexe"
package.name = "tankwar"
package.language = "c++"
package.bindir = "../../bin"

package.config["Debug"].objdir = "./" .. package.name .. "/Debug"
package.config["Debug"].target = package.name .. "_d"
package.config["Release"].objdir = "./" .. package.name .. "/Release"
package.config["Release"].target = package.name

package.linkoptions ={ "/NODEFAULTLIB:libc" }
package.config["Release"].buildflags = {"optimize-speed", "no-symbols" }
package.buildflags = {"no-main", "extra-warnings", "static-runtime", "no-exceptions", "no-rtti" }

if (target == "vs2005") then
	package.defines = { "_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE" }
end

package.config["Debug"].defines = { "WIN32", "_DEBUG", "_WINDOWS", "USE_PROFILER=1" }
package.config["Debug"].links = { "hge", "hgehelp", "curvedani_d", "hgeport_d" }
package.config["Release"].defines = { "WIN32", "NDEBUG", "_WINDOWS", "USE_PROFILER=1" }
package.config["Release"].links = { "hge", "hgehelp", "curvedani", "hgeport" }
package.includepaths = { "../../include", "../../include/ca", "../../include/hge" }

package.libpaths = { "../../lib", "../../lib/" .. target } 

package.files = {
  matchrecursive("../../include/*.h", "../../src/*.cpp", "../../src/*.h")
}



-----------------------------
-- PathFinding package
-----------------------------
package = packagePathFinding
package.path = "../build/" .. target
package.kind = "winexe"
package.name = "pathFinding"
package.language = "c++"
package.bindir = "../../bin"

package.config["Debug"].objdir = "./" .. package.name .. "/Debug"
package.config["Debug"].target = package.name .. "_d"
package.config["Release"].objdir = "./" .. package.name .. "/Release"
package.config["Release"].target = package.name

package.linkoptions ={ "/NODEFAULTLIB:libc" }
package.config["Release"].buildflags = {"optimize-speed", "no-symbols" }
package.buildflags = {"no-main", "extra-warnings", "static-runtime", "no-exceptions", "no-rtti" }

if (target == "vs2005") then
	package.defines = { "_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE" }
end

package.config["Debug"].defines = { "WIN32", "_DEBUG", "_WINDOWS", "USE_PROFILER=0", "PATH_FINDING_TOOL=1" }
package.config["Debug"].links = { "hge", "hgehelp", "curvedani_d", "hgeport_d" }
package.config["Release"].defines = { "WIN32", "NDEBUG", "_WINDOWS", "USE_PROFILER=0", "PATH_FINDING_TOOL=1" }
package.config["Release"].links = { "hge", "hgehelp", "curvedani", "hgeport" }
package.includepaths = { "../../include", "../../include/ca", "../../include/hge" }

package.libpaths = { "../../lib", "../../lib/" .. target } 

package.files = {
  matchrecursive("../../include/*.h", "../../src/*.cpp", "../../src/*.h"),
  matchrecursive("../../tools/*.h", "../../tools/*.cpp")
}
package.excludes = {
  "../../src/common/main.cpp",
  "../../include/game/mainGameState.h",
  "../../src/game/mainGameState.cpp",
  matchrecursive("../../src/menu/*.*"),
}

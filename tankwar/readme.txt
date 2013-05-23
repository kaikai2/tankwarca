Tankwar项目是我在空闲时间开发的坦克大战游戏。

svn仓库结构
===========
tankwar/bin	资源以及可执行文件生成路径
tankwar/build	用于存放通过premake生成的项目文件以及生成项目时产生临时文件
tankwar/include	公共头文件
tankwar/lib	公共库文件
tankwar/premake	premake脚本以及premake程序
tankwar/src	源代码

如何生成项目文件
================
本项目使用premake生成项目文件。请运行tankwar/premake/*.bat生成对应你开发环境的项目文件。项目文件将生成到tankwar/build/目录下。
更多关于premake的说明，请参阅tankwar/premake/INSTALL.txt（感谢gamcat编写该说明文件）

如何获得所需要的库
==================
本项目依赖curvedani以及hgeport库，请从
http://code.google.com/p/curvedani/source
获得该库源代码并生成得到库文件：
curvedani.lib
curvedani_d.lib
hgeport.lib
hgeport_d.lib
请将这些文件复制到tankwar/lib/下对应开发环境的目录下。如tankwar/lib/vs2005
本项目依赖hge库，该库所需文件已经包含在本项目svn仓库内。不需要另外获取。目前版本使用的hge库版本号为1.7。
本项目附带verlet库，该库从gamedev.net一个名叫verlet的物理演示示例程序修改而来。源代码已经包含在本项目svn仓库内。

源代码目录结构
==============
include/ca	curvedani库以及其对应的hgeport头文件
include/hge	hge库头文件
include/verlet	verlet物理模拟库
include/common	公共头文件
include/game	游戏界面的头文件
include/menu	菜单界面的头文件
src/verlet	verlet物理模拟库源文件
src/common	公共源文件
src/game	游戏界面的源文件
src/menu	菜单界面的源文件

源代码介绍
==========

未完待续

kaikai
2008-2-26
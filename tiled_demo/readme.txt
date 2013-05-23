Tiled_demo项目是我在空闲时间开发的tiled地图演示程序。

svn仓库结构
===========
tiled_demo/bin	资源以及可执行文件生成路径
tiled_demo/build	用于存放通过premake生成的项目文件以及生成项目时产生临时文件
tiled_demo/include	公共头文件
tiled_demo/lib	公共库文件
tiled_demo/premake	premake脚本以及premake程序
tiled_demo/src	源代码

如何生成项目文件
================
本项目使用premake生成项目文件。请运行tiled_demo/premake/*.bat生成对应你开发环境的项目文件。项目文件将生成到tiled_demo/build/目录下。
更多关于premake的说明，请参阅tiled_demo/premake/INSTALL.txt（感谢gamcat编写该说明文件）

如何获得所需要的库
==================
本项目依赖curvedani以及hgeport库，请从
http://code.google.com/p/curvedani/source
获得该库源代码并生成得到库文件：
curvedani.lib
curvedani_d.lib
hgeport.lib
hgeport_d.lib
请将这些文件复制到tiled_demo/lib/下对应开发环境的目录下。如tiled_demo/lib/vs2005
本项目依赖hge库，该库所需文件已经包含在本项目svn仓库内。不需要另外获取。目前版本使用的hge库版本号为1.7。

源代码目录结构
==============
include/ca	curvedani库以及其对应的hgeport头文件
include/hge	hge库头文件
include/common	公共头文件
include/game	游戏界面的头文件
include/menu	菜单界面的头文件
src/common	公共源文件
src/game	游戏界面的源文件
src/menu	菜单界面的源文件

源代码介绍
==========

未完待续

kaikai
2008-7-6
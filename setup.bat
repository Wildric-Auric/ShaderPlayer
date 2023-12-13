
git submodule update
move NWengineCore Content
mkdir Content\vendor
move imgui Content\vendor
move glfw  Content\vendor

del  Content\src\main.cpp
move *.cpp Content\src
move *.h   Content\src
move premake5.lua Content\src
move Shaders Content\src
cd Content\src
premake5 vs2022

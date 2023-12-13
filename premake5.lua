workspace "ShaderPlayer"
	configurations {"Release", "Debug"}
	architecture "x86"
	language "C++"
    cppdialect "C++11"
	targetdir "Bin/%{prj.name}/%{cfg.buildcfg}"
    objdir    "Bin/objs"
    characterset("MBCS")
    buildoptions { "/EHsc" }
	location "./"
	project "ShaderPlayer"
	kind "ConsoleApp"
	includedirs {
		"%{wks.location}/../include/",
		"../vendor/imgui/",
		"../vendor/imgui/backends/",
		"../vendor/glfw/include/"
	}

	files {
		"*.cpp",
		"../vendor/imgui/*.cpp",
		"../vendor/imgui/backends/imgui_impl_glfw.cpp",
		"../vendor/imgui/backends/imgui_impl_opengl3.cpp",
		"../vendor/imgui/backends/imgui_impl_win32.cpp"
	}

	filter "configurations:Release"
		optimize "On"
		links {
			"%{wks.location}/../lib/NWengineCore.lib"
		}
	filter "configurations:Debug" 
		symbols "On"
		links {
			"%{wks.location}/../lib/debug/NWengineCore_d.lib"
		}

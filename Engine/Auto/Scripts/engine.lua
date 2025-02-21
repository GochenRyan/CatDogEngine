--------------------------------------------------------------
-- @Description : Makefile of CatDogEngine Runtime
--------------------------------------------------------------

project("Engine")
	kind(EngineBuildLibKind)
	language("C++")
	cppdialect("C++latest")
	dependson { "bx", "bimg", "bimg_decode", "bgfx" } -- sdl is pre-built in makefile.
	
	location(path.join(IntermediatePath, "Engine/Runtime"))
	targetdir(BinariesPath)

	files {
		path.join(RuntimeSourcePath, "**.*"),
		path.join(ThirdPartySourcePath, "rapidxml/**.hpp"),
		path.join(ThirdPartySourcePath, "imgui/*.h"),
		path.join(ThirdPartySourcePath, "imgui/*.cpp"),
		path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
		path.join(ThirdPartySourcePath, "spdlog/include/spdlog/**.*"),
	}
	
	vpaths {
		["Source/*"] = { 
			path.join(RuntimeSourcePath, "**.*"),
		},
		["ImGui"] = {
			path.join(ThirdPartySourcePath, "imgui/*.h"),
			path.join(ThirdPartySourcePath, "imgui/*.cpp"),
			path.join(ThirdPartySourcePath, "imgui/misc/freetype/imgui_freetype.*"),
		},
	}
	
	local bgfxBuildBinPath = nil
	local platformDefines = nil
	local platformIncludeDirs = nil

	filter { "system:windows" }
		bgfxBuildBinPath = ThirdPartySourcePath.."/bgfx/.build/win64_"..IDEConfigs.BuildIDEName.."/bin"
		platformIncludeDirs = { 
			path.join(ThirdPartySourcePath, "bx/include/compat/msvc")
		}
	filter {}

	includedirs {
		RuntimeSourcePath,
		ThirdPartySourcePath,
		path.join(ThirdPartySourcePath, "AssetPipeline/public"),
		path.join(ThirdPartySourcePath, "bgfx/include"),
		path.join(ThirdPartySourcePath, "bgfx/3rdparty"),
		path.join(ThirdPartySourcePath, "bimg/include"),
		path.join(ThirdPartySourcePath, "bimg/3rdparty"),
		path.join(ThirdPartySourcePath, "bx/include"),
		path.join(ThirdPartySourcePath, "sdl/include"),
		path.join(ThirdPartySourcePath, "imgui"),
		path.join(ThirdPartySourcePath, "freetype/include"),
		table.unpack(platformIncludeDirs),
		path.join(EnginePath, "BuiltInShaders/UniformDefines"),
		path.join(ThirdPartySourcePath, "spdlog/include"),
	}

	filter { "configurations:Debug" }
		platformDefines = {
			"BX_CONFIG_DEBUG",
		}
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Debug"),
			path.join(ThirdPartySourcePath, "freetype/build/Debug"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2d", "sdl2maind",
			"bgfxDebug", "bimgDebug", "bxDebug", "bimg_decodeDebug",
			"freetyped"
		}
	filter { "configurations:Release" }
		libdirs {
			path.join(ThirdPartySourcePath, "sdl/build/Release"),
			path.join(ThirdPartySourcePath, "freetype/build/Release"),
			bgfxBuildBinPath,
		}
		links {
			"sdl2", "sdl2main",
			"bgfxRelease", "bimgRelease", "bxRelease", "bimg_decodeRelease",
			"freetype"
		}
	filter {}

	if "SharedLib" == EngineBuildLibKind then
		table.insert(platformDefines, "ENGINE_BUILD_SHARED")
	end

	local engineBuiltInShaderPath = RootPath.."/Engine/BuiltInShaders/shaders/"
	local editorResourcesPath = RootPath.."/Engine/EditorResources/"
	local projectResourcesPath = RootPath.."/Projects/PBRViewer/Resources/"
	defines {
		"SDL_MAIN_HANDLED", -- don't use SDL_main() as entry point
		"__STDC_LIMIT_MACROS", "__STDC_FORMAT_MACROS", "__STDC_CONSTANT_MACROS",
		"STB_IMAGE_STATIC",
		"IMGUI_ENABLE_FREETYPE",
		"SPDLOG_NO_EXCEPTIONS", "FMT_USE_NONTYPE_TEMPLATE_ARGS=0",
		GetPlatformMacroName(),
		table.unpack(platformDefines),
		"CDENGINE_BUILTIN_SHADER_PATH=\""..engineBuiltInShaderPath.."\"",
		"CDENGINE_RESOURCES_ROOT_PATH=\""..projectResourcesPath.."\"",
		"CDEDITOR_RESOURCES_ROOT_PATH=\""..editorResourcesPath.."\"",
	}

	-- use /MT /MTd, not /MD /MDd
	staticruntime "on"
	filter { "configurations:Debug" }
		runtime "Debug" -- /MTd
	filter { "configurations:Release" }
		runtime "Release" -- /MT
	filter {}

	-- Disable these options can reduce the size of compiled binaries.
	justmycode("Off")
	editAndContinue("Off")
	exceptionhandling("Off")
	rtti("Off")
		
	-- Strict.
	warnings("Default")
	externalwarnings("Off")
	
	flags {
		"FatalWarnings", -- treat warnings as errors
		"MultiProcessorCompile", -- compiler uses multiple thread
	}
	
	-- copy dll into binary folder automatically.
	filter { "configurations:Debug" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Debug/SDL2d.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/GenericProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/TerrainProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/CDConsumer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Debug/assimp-*-mtd.*").."\" \""..BinariesPath.."\"",
		}
	filter { "configurations:Release" }
		postbuildcommands {
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "sdl/build/Release/SDL2.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/AssetPipelineCore.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/GenericProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/TerrainProducer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/CDConsumer.*").."\" \""..BinariesPath.."\"",
			"{COPYFILE} \""..path.join(ThirdPartySourcePath, "AssetPipeline/build/bin/Release/assimp-*-mt.*").."\" \""..BinariesPath.."\"",
		}
	filter {}
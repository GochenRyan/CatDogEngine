#pragma once 

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

namespace engine
{

class Path
{
public:
	static constexpr size_t MAX_PATH_SIZE = 1024;
	static constexpr const char* EngineName = "CatDogEngine";
	static constexpr const char* ShaderInputExtension = ".sc";
	static constexpr const char* ShaderOutputExtension = ".bin";

	static std::optional<std::filesystem::path> GetApplicationDataPath();

	static std::filesystem::path GetEngineBuiltinShaderPath();
	static std::filesystem::path GetEngineResourcesPath();
	static std::filesystem::path GetEditorResourcesPath();

	static std::string GetBuiltinShaderInputPath(const char* pShaderName);
	static std::string GetShaderOutputPath(const char* pInputFilePath, const std::string& options = "");
	static std::string GetTextureOutputFilePath(const char* pInputFilePath, const char* extension);

private:
	static const char* GetPlatformPathKey();
	static std::filesystem::path GetPlatformAppDataPath(char(&value)[MAX_PATH_SIZE]);
};

}
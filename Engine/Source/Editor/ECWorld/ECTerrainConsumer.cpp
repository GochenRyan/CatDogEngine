#include "ECTerrainConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Node.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"
#include "Utilities/StringUtils.h"

#include <cassert>
#include <filesystem>

namespace editor
{

ECTerrainConsumer::ECTerrainConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext)
	: m_pSceneWorld(pSceneWorld)
	, m_pRenderContext(pRenderContext)
{
}

void ECTerrainConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	assert(pSceneDatabase->GetMeshCount() > 0);
	
	for (const cd::Mesh& mesh : pSceneDatabase->GetMeshes())
	{
		const cd::MeshID::ValueType meshID = mesh.GetID().Data();
		if (m_meshToEntity.find(meshID) == m_meshToEntity.cend())
		{
			engine::Entity terrainEntity = m_pSceneWorld->GetWorld()->CreateEntity();
			engine::MaterialType* pMaterialType = m_pSceneWorld->GetTerrainMaterialType();
			AddStaticMesh(terrainEntity, &mesh, pMaterialType->GetRequiredVertexFormat());
			const cd::MaterialID meshMaterialID = mesh.GetMaterialID();
			if (meshMaterialID.IsValid())
			{
				AddMaterial(terrainEntity, &pSceneDatabase->GetMaterial(meshMaterialID.Data()), pMaterialType, pSceneDatabase);
			}
			m_meshToEntity[meshID] = terrainEntity;
		}
	}
}

void ECTerrainConsumer::Clear() 
{
	for (const auto& [meshID, entity] : m_meshToEntity) 
	{
		m_pSceneWorld->DeleteEntity(entity);
	}
}

void ECTerrainConsumer::AddStaticMesh(engine::Entity entity, const cd::Mesh* mesh, const cd::VertexFormat& vertexFormat)
{
	assert(mesh->GetVertexCount() > 0 && mesh->GetPolygonCount() > 0);

	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::NameComponent& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(mesh->GetName());

	engine::StaticMeshComponent& staticMeshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
	staticMeshComponent.SetMeshData(mesh);
	staticMeshComponent.SetRequiredVertexFormat(&vertexFormat);
	staticMeshComponent.Build();
}

void ECTerrainConsumer::AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase)
{
	const std::optional<cd::TextureID> optBaseColorTexture = pMaterial->GetTextureID(cd::MaterialTextureType::BaseColor);
	assert(optBaseColorTexture.has_value());
	const cd::Texture& baseColorTexture = pSceneDatabase->GetTexture(optBaseColorTexture.value().Data());
	const std::string baseColorTexturePath = GetTextureOutputFilePath(baseColorTexture.GetPath());
	std::string textureDir = cd::string_format("%sTextures/textures/%s.png", CDENGINE_RESOURCES_ROOT_PATH, baseColorTexture.GetPath());
	ResourceBuilder::Get().AddTextureBuildTask(baseColorTexture.GetType(), textureDir.c_str(), baseColorTexturePath.c_str());
	
	// Shaders
	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();
	const std::string outputVSFilePath = GetShaderOutputFilePath(shaderSchema.GetVertexShaderPath());
	ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Vertex, shaderSchema.GetVertexShaderPath(), outputVSFilePath.c_str());

	const std::string outputFSFilePath = GetShaderOutputFilePath(shaderSchema.GetFragmentShaderPath());
	ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment, shaderSchema.GetFragmentShaderPath(), outputFSFilePath.c_str());
	engine::StringCrc currentUberOption = engine::ShaderSchema::DefaultUberOption;

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	// TODO : create material component before ResourceBuilder done.
	engine::MaterialComponent& materialComponent = m_pSceneWorld->GetWorld()->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.SetMaterialData(pMaterial);
	materialComponent.SetMaterialType(pMaterialType);
	materialComponent.SetUberShaderOption(currentUberOption);

	// Textures
	materialComponent.AddTextureFileBlob(baseColorTexture.GetType(), ResourceLoader::LoadTextureFile(baseColorTexturePath.c_str()));
	const std::optional<cd::TextureID> optElevationTexture = pMaterial->GetTextureID(cd::MaterialTextureType::Roughness);
	assert(optElevationTexture.has_value());
	// Don't need to load as this is generated
	const cd::Texture& elevationTexture = pSceneDatabase->GetTexture(optElevationTexture.value().Data());
	materialComponent.AddTextureBlob(elevationTexture.GetType(), elevationTexture.GetTextureFormat(), engine::MaterialComponent::TextureBlob(elevationTexture.GetRawTexture()), elevationTexture.GetWidth(), elevationTexture.GetHeight());

	// Shaders
	shaderSchema.AddUberOptionVSBlob(ResourceLoader::LoadShader(outputVSFilePath.c_str()));
	const engine::ShaderSchema::ShaderBlob& VSBlob = shaderSchema.GetVSBlob();
	bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(VSBlob.data(), static_cast<uint32_t>(VSBlob.size())));

	shaderSchema.AddUberOptionFSBlob(engine::ShaderSchema::DefaultUberOption, ResourceLoader::LoadShader(outputFSFilePath.c_str()));
	const engine::ShaderSchema::ShaderBlob& FSBlob = shaderSchema.GetFSBlob(engine::ShaderSchema::DefaultUberOption);
	bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(FSBlob.data(), static_cast<uint32_t>(FSBlob.size())));
	bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
	shaderSchema.SetCompiledProgram(engine::ShaderSchema::DefaultUberOption, uberProgramHandle.idx);

	materialComponent.Build();
}

std::string ECTerrainConsumer::GetShaderOutputFilePath(const char* pInputFilePath, const char* pAppendFileName /* = nullptr */)
{
	std::filesystem::path inputShaderPath(pInputFilePath);
	std::string inputShaderFileName = inputShaderPath.stem().generic_string();
	std::string outputShaderPath = CDENGINE_RESOURCES_ROOT_PATH;
	outputShaderPath += "Shaders/" + inputShaderFileName;
	if (pAppendFileName)
	{
		if (engine::ShaderSchema::DefaultUberOption != engine::StringCrc(pAppendFileName))
		{
			outputShaderPath += "_";
			outputShaderPath += pAppendFileName;
		}
	}
	outputShaderPath += ".bin";
	return outputShaderPath;
}


std::string ECTerrainConsumer::GetTextureOutputFilePath(const char* pInputFilePath)
{
	std::filesystem::path inputTexturePath(pInputFilePath);
	std::string inputTextureFileName = inputTexturePath.stem().generic_string();
	std::string outputTexturePath = CDENGINE_RESOURCES_ROOT_PATH;
	outputTexturePath += "Textures/" + inputTextureFileName + ".dds";
	return outputTexturePath;
}

}
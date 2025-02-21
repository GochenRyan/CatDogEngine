#include "SceneWorld.h"

#include "Log/Log.h"
#include "Path/Path.h"

namespace engine
{

SceneWorld::SceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();
	m_pAnimationStorage = m_pWorld->Register<engine::AnimationComponent>();
	m_pCameraStorage = m_pWorld->Register<engine::CameraComponent>();
	m_pCollisionMeshStorage = m_pWorld->Register<engine::CollisionMeshComponent>();
	m_pHierarchyStorage = m_pWorld->Register<engine::HierarchyComponent>();
	m_pLightStorage = m_pWorld->Register<engine::LightComponent>();
	m_pMaterialStorage = m_pWorld->Register<engine::MaterialComponent>();
	m_pNameStorage = m_pWorld->Register<engine::NameComponent>();
	m_pSkyStorage = m_pWorld->Register<engine::SkyComponent>();
	m_pStaticMeshStorage = m_pWorld->Register<engine::StaticMeshComponent>();
	m_pTransformStorage = m_pWorld->Register<engine::TransformComponent>();

	CreatePBRMaterialType();
	CreateAnimationMaterialType();
	CreateTerrainMaterialType();
}

void SceneWorld::CreatePBRMaterialType()
{
	m_pPBRMaterialType = std::make_unique<MaterialType>();
	m_pPBRMaterialType->SetMaterialName("CD_PBR");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_PBR"), Path::GetBuiltinShaderInputPath("fs_PBR"));
	shaderSchema.RegisterUberOption(Uber::ALBEDO);
	shaderSchema.RegisterUberOption(Uber::NORMAL_MAP);
	shaderSchema.RegisterUberOption(Uber::OCCLUSION);
	shaderSchema.RegisterUberOption(Uber::ROUGHNESS);
	shaderSchema.RegisterUberOption(Uber::METALLIC);
	shaderSchema.RegisterUberOption(Uber::IBL);
	// Technically, option LoadingStatus:: is an actual shader.
	// We can use AddSingleUberOption to add it to shaderSchema,
	// whithout combine with any other option.
	shaderSchema.AddSingleUberOption(LoadingStatus::MISSING_RESOURCES, Path::GetBuiltinShaderInputPath("fs_unlit_flat_red"));
	shaderSchema.AddSingleUberOption(LoadingStatus::LOADING_SHADERS, Path::GetBuiltinShaderInputPath("fs_unlit_flat_blue"));
	shaderSchema.AddSingleUberOption(LoadingStatus::LOADING_TEXTURES, Path::GetBuiltinShaderInputPath("fs_unlit_flat_green"));
	shaderSchema.AddSingleUberOption(LoadingStatus::LOADING_ERROR, Path::GetBuiltinShaderInputPath("fs_unlit_flat_pink"));
	m_pPBRMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pPBRMaterialType->SetRequiredVertexFormat(cd::MoveTemp(pbrVertexFormat));

	// Slot index should align to shader codes.
	// We want basic PBR materials to be flexible.
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::BaseColor, 0);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Normal, 1);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Occlusion, 2);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Roughness, 2);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Metallic, 2);
	//m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Emissive, );
}

void SceneWorld::CreateAnimationMaterialType()
{
	m_pAnimationMaterialType = std::make_unique<MaterialType>();
	m_pAnimationMaterialType->SetMaterialName("CD_Animation");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_animation"), Path::GetBuiltinShaderInputPath("fs_animation"));
	m_pAnimationMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat animationVertexFormat;
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	//animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneIndex, cd::AttributeValueType::Int16, 4U);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 4U);
	m_pAnimationMaterialType->SetRequiredVertexFormat(cd::MoveTemp(animationVertexFormat));
}

void SceneWorld::CreateTerrainMaterialType()
{
	m_pTerrainMaterialType = std::make_unique<MaterialType>();
	m_pTerrainMaterialType->SetMaterialName("CD_Terrain");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_terrain"), Path::GetBuiltinShaderInputPath("fs_terrain"));
	shaderSchema.RegisterUberOption(Uber::DEFAULT);
	m_pTerrainMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat terrainVertexFormat;
	terrainVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	terrainVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pTerrainMaterialType->SetRequiredVertexFormat(cd::MoveTemp(terrainVertexFormat));

	m_pTerrainMaterialType->AddRequiredTextureType(cd::MaterialTextureType::BaseColor, 0);
	m_pTerrainMaterialType->AddRequiredTextureType(cd::MaterialTextureType::Roughness, 1);
}

void SceneWorld::SetSelectedEntity(engine::Entity entity)
{
	CD_TRACE("Select entity : {0}", entity);
	m_selectedEntity = entity;
}

void SceneWorld::SetMainCameraEntity(engine::Entity entity)
{
	CD_TRACE("Setup main camera entity : {0}", entity);
	m_mainCameraEntity = entity;
}

void SceneWorld::OnResizeSceneView(uint16_t width, uint16_t height)
{
	if (engine::CameraComponent* pCameraComponent = GetCameraComponent(GetMainCameraEntity()))
	{
		pCameraComponent->SetAspect(width, height);
	}
}

}
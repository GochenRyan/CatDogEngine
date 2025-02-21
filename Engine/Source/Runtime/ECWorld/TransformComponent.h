#pragma once

#include "Core/StringCrc.h"
#include "Math/Transform.hpp"

namespace engine
{

class TransformComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("TransformComponent");
		return className;
	}

public:
	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent& operator=(const TransformComponent&) = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator=(TransformComponent&&) = default;
	~TransformComponent() = default;

	const cd::Transform& GetTransform() const { return m_transform; }
	cd::Transform& GetTransform() { return m_transform; }
	void SetTransform(cd::Transform transform) { m_transform = cd::MoveTemp(transform); m_isMatrixDirty = true;  }

	const cd::Matrix4x4& GetWorldMatrix() const { return m_localToWorldMatrix; }

	void Dirty() const { m_isMatrixDirty = true; }

	void Reset();
	void Build();

private:
	// Input
	cd::Transform m_transform;

	// Status
	mutable bool m_isMatrixDirty;

	// Output
	cd::Matrix4x4 m_localToWorldMatrix;
};

}
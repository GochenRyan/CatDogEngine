#pragma once

#include <bgfx/bgfx.h>

struct Uniforms {
	// remember to set the size of array in shader as same as this.
	static constexpr uint16_t vec4Count = 7;

	void Init() {
		u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, vec4Count);
	}

	void Submit() {
		int index = 0;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = -2.0f;
		basicParams[index++] = -2.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +1.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;
		basicParams[index++] = +0.0f;

		bgfx::setUniform(u_params, basicParams, vec4Count);
	}

	void Destory() {
		bgfx::destroy(u_params);
	}

	union {
		struct {
			union {
				float m_mtx[16];
				/*0*/ struct { float m_mtx0[4]; };
				/*1*/ struct { float m_mtx1[4]; };
				/*2*/ struct { float m_mtx2[4]; };
				/*3*/ struct { float m_mtx3[4]; };
			};
		};

		float basicParams[4 * vec4Count];
	};

	bgfx::UniformHandle u_params;
};
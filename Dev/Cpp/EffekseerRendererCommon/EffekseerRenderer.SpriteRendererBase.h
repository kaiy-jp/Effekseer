﻿
#ifndef __EFFEKSEERRENDERER_SPRITE_RENDERER_BASE_H__
#define __EFFEKSEERRENDERER_SPRITE_RENDERER_BASE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include <Effekseer.Internal.h>
#include <Effekseer.h>
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "EffekseerRenderer.CommonUtils.h"
#include "EffekseerRenderer.IndexBufferBase.h"
#include "EffekseerRenderer.RenderStateBase.h"
#include "EffekseerRenderer.StandardRenderer.h"
#include "EffekseerRenderer.VertexBufferBase.h"

//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
namespace EffekseerRenderer
{
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
typedef ::Effekseer::SpriteRenderer::NodeParameter efkSpriteNodeParam;
typedef ::Effekseer::SpriteRenderer::InstanceParameter efkSpriteInstanceParam;
typedef ::Effekseer::Vec3f efkVector3D;

template <typename RENDERER, typename VERTEX_NORMAL, typename VERTEX_DISTORTION>
class SpriteRendererBase : public ::Effekseer::SpriteRenderer, public ::Effekseer::AlignedAllocationPolicy<16>
{
protected:
	RENDERER* m_renderer;
	int32_t m_spriteCount;
	uint8_t* m_ringBufferData;

	struct KeyValue
	{
		float Key;
		efkSpriteInstanceParam Value;
	};

	Effekseer::CustomAlignedVector<KeyValue> instances;
	int32_t vertexCount_ = 0;
	int32_t stride_ = 0;
	int32_t customData1Count_ = 0;
	int32_t customData2Count_ = 0;

public:
	SpriteRendererBase(RENDERER* renderer)
		: m_renderer(renderer)
		, m_spriteCount(0)
		, m_ringBufferData(nullptr)
	{
		// reserve buffers
		instances.reserve(m_renderer->GetSquareMaxCount());
	}

	virtual ~SpriteRendererBase()
	{
	}

protected:
	void RenderingInstance(const efkSpriteInstanceParam& inst,
						   const efkSpriteNodeParam& param,
						   const StandardRendererState& state,
						   const ::Effekseer::Mat44f& camera)
	{
		if (state.MaterialPtr != nullptr && !state.MaterialPtr->IsSimpleVertex)
		{
			Rendering_Internal<DynamicVertex>(param, inst, nullptr, camera);
		}
		else if (param.BasicParameterPtr->MaterialType == Effekseer::RendererMaterialType::Lighting)
		{
			Rendering_Internal<LightingVertex>(param, inst, nullptr, camera);
		}
		else if (param.BasicParameterPtr->MaterialType == Effekseer::RendererMaterialType::BackDistortion)
		{
			Rendering_Internal<VERTEX_DISTORTION>(param, inst, nullptr, camera);
		}
		else
		{
			Rendering_Internal<VERTEX_NORMAL>(param, inst, nullptr, camera);
		}
	}

	void BeginRendering_(RENDERER* renderer, int32_t count, const efkSpriteNodeParam& param)
	{
		EffekseerRenderer::StandardRendererState state;
		state.AlphaBlend = param.BasicParameterPtr->AlphaBlend;
		state.CullingType = ::Effekseer::CullingType::Double;
		state.DepthTest = param.ZTest;
		state.DepthWrite = param.ZWrite;
		state.TextureFilter1 = param.BasicParameterPtr->TextureFilter1;
		state.TextureWrap1 = param.BasicParameterPtr->TextureWrap1;
		state.TextureFilter2 = param.BasicParameterPtr->TextureFilter2;
		state.TextureWrap2 = param.BasicParameterPtr->TextureWrap2;
#ifdef __EFFEKSEER_BUILD_VERSION16__
		state.TextureFilter3 = param.BasicParameterPtr->TextureFilter3;
		state.TextureWrap3 = param.BasicParameterPtr->TextureWrap3;
		state.TextureFilter4 = param.BasicParameterPtr->TextureFilter4;
		state.TextureWrap4 = param.BasicParameterPtr->TextureWrap4;
		state.TextureFilter5 = param.BasicParameterPtr->TextureFilter5;
		state.TextureWrap5 = param.BasicParameterPtr->TextureWrap5;
		state.TextureFilter6 = param.BasicParameterPtr->TextureFilter6;
		state.TextureWrap6 = param.BasicParameterPtr->TextureWrap6;
		state.TextureFilter7 = param.BasicParameterPtr->TextureFilter7;
		state.TextureWrap7 = param.BasicParameterPtr->TextureWrap7;

		state.EnableInterpolation = param.BasicParameterPtr->EnableInterpolation;
		state.UVLoopType = param.BasicParameterPtr->UVLoopType;
		state.InterpolationType = param.BasicParameterPtr->InterpolationType;
		state.FlipbookDivideX = param.BasicParameterPtr->FlipbookDivideX;
		state.FlipbookDivideY = param.BasicParameterPtr->FlipbookDivideY;

		state.UVDistortionIntensity = param.BasicParameterPtr->UVDistortionIntensity;

		state.TextureBlendType = param.BasicParameterPtr->TextureBlendType;

		state.BlendUVDistortionIntensity = param.BasicParameterPtr->BlendUVDistortionIntensity;

		state.EmissiveScaling = param.BasicParameterPtr->EmissiveScaling;

		state.EdgeThreshold = param.BasicParameterPtr->EdgeThreshold;
		state.EdgeColor[0] = param.BasicParameterPtr->EdgeColor[0];
		state.EdgeColor[1] = param.BasicParameterPtr->EdgeColor[1];
		state.EdgeColor[2] = param.BasicParameterPtr->EdgeColor[2];
		state.EdgeColor[3] = param.BasicParameterPtr->EdgeColor[3];
		state.EdgeColorScaling = param.BasicParameterPtr->EdgeColorScaling;
#endif

		state.Distortion = param.BasicParameterPtr->MaterialType == Effekseer::RendererMaterialType::BackDistortion;
		state.DistortionIntensity = param.BasicParameterPtr->DistortionIntensity;
		state.MaterialType = param.BasicParameterPtr->MaterialType;

		state.CopyMaterialFromParameterToState(param.EffectPointer,
											   param.BasicParameterPtr->MaterialParameterPtr,
											   param.BasicParameterPtr->Texture1Index,
											   param.BasicParameterPtr->Texture2Index
#ifdef __EFFEKSEER_BUILD_VERSION16__
											   ,
											   param.BasicParameterPtr->Texture3Index,
											   param.BasicParameterPtr->Texture4Index,
											   param.BasicParameterPtr->Texture5Index,
											   param.BasicParameterPtr->Texture6Index,
											   param.BasicParameterPtr->Texture7Index
#endif
		);
		customData1Count_ = state.CustomData1Count;
		customData2Count_ = state.CustomData2Count;

		renderer->GetStandardRenderer()->UpdateStateAndRenderingIfRequired(state);

		renderer->GetStandardRenderer()->BeginRenderingAndRenderingIfRequired(count * 4, stride_, (void*&)m_ringBufferData);
		m_spriteCount = 0;

		vertexCount_ = count * 4;

		instances.clear();
	}

	void Rendering_(const efkSpriteNodeParam& parameter,
					const efkSpriteInstanceParam& instanceParameter,
					void* userData,
					const ::Effekseer::Mat44f& camera)
	{
		if (parameter.ZSort == Effekseer::ZSortType::None)
		{
			auto camera = m_renderer->GetCameraMatrix();
			const auto& state = m_renderer->GetStandardRenderer()->GetState();

			RenderingInstance(instanceParameter, parameter, state, camera);
		}
		else
		{
			KeyValue kv;
			kv.Value = instanceParameter;
			instances.push_back(kv);
		}
	}

	enum class VertexType
	{
		Normal,
		Distortion,
		Dynamic,
		Lightning,
	};

	VertexType GetVertexType(const VERTEX_NORMAL* v)
	{
		return VertexType::Normal;
	}

	VertexType GetVertexType(const VERTEX_DISTORTION* v)
	{
		return VertexType::Distortion;
	}

	VertexType GetVertexType(const DynamicVertex* v)
	{
		return VertexType::Dynamic;
	}

	VertexType GetVertexType(const LightingVertex* v)
	{
		return VertexType::Lightning;
	}

	template <typename VERTEX>
	void Rendering_Internal(const efkSpriteNodeParam& parameter,
							const efkSpriteInstanceParam& instanceParameter,
							void* userData,
							const ::Effekseer::Mat44f& camera)
	{
		if (m_ringBufferData == nullptr)
			return;

		StrideView<VERTEX> verteies(m_ringBufferData, stride_, 4);

		auto vertexType = GetVertexType((VERTEX*)m_ringBufferData);

		for (int i = 0; i < 4; i++)
		{
			verteies[i].Pos.X = instanceParameter.Positions[i].GetX();
			verteies[i].Pos.Y = instanceParameter.Positions[i].GetY();
			verteies[i].Pos.Z = 0.0f;

			verteies[i].SetColor(instanceParameter.Colors[i]);

#ifdef __EFFEKSEER_BUILD_VERSION16__
			verteies[i].SetFlipbookIndexAndNextRate(instanceParameter.FlipbookIndexAndNextRate);
			verteies[i].SetAlphaThreshold(instanceParameter.AlphaThreshold);
#endif
		}

		verteies[0].UV[0] = instanceParameter.UV.X;
		verteies[0].UV[1] = instanceParameter.UV.Y + instanceParameter.UV.Height;

		verteies[1].UV[0] = instanceParameter.UV.X + instanceParameter.UV.Width;
		verteies[1].UV[1] = instanceParameter.UV.Y + instanceParameter.UV.Height;

		verteies[2].UV[0] = instanceParameter.UV.X;
		verteies[2].UV[1] = instanceParameter.UV.Y;

		verteies[3].UV[0] = instanceParameter.UV.X + instanceParameter.UV.Width;
		verteies[3].UV[1] = instanceParameter.UV.Y;

#ifdef __EFFEKSEER_BUILD_VERSION16__
		verteies[0].SetAlphaUV(instanceParameter.AlphaUV.X, 0);
		verteies[0].SetAlphaUV(instanceParameter.AlphaUV.Y + instanceParameter.AlphaUV.Height, 1);

		verteies[1].SetAlphaUV(instanceParameter.AlphaUV.X + instanceParameter.AlphaUV.Width, 0);
		verteies[1].SetAlphaUV(instanceParameter.AlphaUV.Y + instanceParameter.AlphaUV.Height, 1);

		verteies[2].SetAlphaUV(instanceParameter.AlphaUV.X, 0);
		verteies[2].SetAlphaUV(instanceParameter.AlphaUV.Y, 1);

		verteies[3].SetAlphaUV(instanceParameter.AlphaUV.X + instanceParameter.AlphaUV.Width, 0);
		verteies[3].SetAlphaUV(instanceParameter.AlphaUV.Y, 1);

		verteies[0].SetUVDistortionUV(instanceParameter.UVDistortionUV.X, 0);
		verteies[0].SetUVDistortionUV(instanceParameter.UVDistortionUV.Y + instanceParameter.UVDistortionUV.Height, 1);

		verteies[1].SetUVDistortionUV(instanceParameter.UVDistortionUV.X + instanceParameter.UVDistortionUV.Width, 0);
		verteies[1].SetUVDistortionUV(instanceParameter.UVDistortionUV.Y + instanceParameter.UVDistortionUV.Height, 1);

		verteies[2].SetUVDistortionUV(instanceParameter.UVDistortionUV.X, 0);
		verteies[2].SetUVDistortionUV(instanceParameter.UVDistortionUV.Y, 1);

		verteies[3].SetUVDistortionUV(instanceParameter.UVDistortionUV.X + instanceParameter.UVDistortionUV.Width, 0);
		verteies[3].SetUVDistortionUV(instanceParameter.UVDistortionUV.Y, 1);

		verteies[0].SetBlendUV(instanceParameter.BlendUV.X, 0);
		verteies[0].SetBlendUV(instanceParameter.BlendUV.Y + instanceParameter.BlendUV.Height, 1);

		verteies[1].SetBlendUV(instanceParameter.BlendUV.X + instanceParameter.BlendUV.Width, 0);
		verteies[1].SetBlendUV(instanceParameter.BlendUV.Y + instanceParameter.BlendUV.Height, 1);

		verteies[2].SetBlendUV(instanceParameter.BlendUV.X, 0);
		verteies[2].SetBlendUV(instanceParameter.BlendUV.Y, 1);

		verteies[3].SetBlendUV(instanceParameter.BlendUV.X + instanceParameter.BlendUV.Width, 0);
		verteies[3].SetBlendUV(instanceParameter.BlendUV.Y, 1);

		verteies[0].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.X, 0);
		verteies[0].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.Y + instanceParameter.BlendAlphaUV.Height, 1);

		verteies[1].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.X + instanceParameter.BlendAlphaUV.Width, 0);
		verteies[1].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.Y + instanceParameter.BlendAlphaUV.Height, 1);

		verteies[2].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.X, 0);
		verteies[2].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.Y, 1);

		verteies[3].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.X + instanceParameter.BlendAlphaUV.Width, 0);
		verteies[3].SetBlendAlphaUV(instanceParameter.BlendAlphaUV.Y, 1);

		verteies[0].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.X, 0);
		verteies[0].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.Y + instanceParameter.BlendUVDistortionUV.Height, 1);

		verteies[1].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.X + instanceParameter.BlendUVDistortionUV.Width, 0);
		verteies[1].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.Y + instanceParameter.BlendUVDistortionUV.Height, 1);

		verteies[2].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.X, 0);
		verteies[2].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.Y, 1);

		verteies[3].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.X + instanceParameter.BlendUVDistortionUV.Width, 0);
		verteies[3].SetBlendUVDistortionUV(instanceParameter.BlendUVDistortionUV.Y, 1);
#endif

		// distortion
		if (vertexType == VertexType::Distortion)
		{
			StrideView<VERTEX_DISTORTION> vs(verteies.pointerOrigin_, stride_, 4);
			for (auto i = 0; i < 4; i++)
			{
				vs[i].Tangent.X = 1.0f;
				vs[i].Tangent.Y = 0.0f;
				vs[i].Tangent.Z = 0.0f;
				vs[i].Binormal.X = 0.0f;
				vs[i].Binormal.Y = 1.0f;
				vs[i].Binormal.Z = 0.0f;
			}
		}
		else if (vertexType == VertexType::Lightning || vertexType == VertexType::Dynamic)
		{
			StrideView<VERTEX> vs(verteies.pointerOrigin_, stride_, 4);
			vs[0].SetUV2(0.0f, 1.0f);
			vs[1].SetUV2(1.0f, 1.0f);
			vs[2].SetUV2(0.0f, 0.0f);
			vs[3].SetUV2(1.0f, 0.0f);
		}

		if (parameter.Billboard == ::Effekseer::BillboardType::Billboard ||
			parameter.Billboard == ::Effekseer::BillboardType::RotatedBillboard ||
			parameter.Billboard == ::Effekseer::BillboardType::YAxisFixed)
		{
			Effekseer::Mat43f mat_rot = Effekseer::Mat43f::Identity;
			Effekseer::Vec3f s;
			Effekseer::Vec3f R;
			Effekseer::Vec3f F;

#ifdef __EFFEKSEER_BUILD_VERSION16__
			if (parameter.EnableViewOffset == true)
			{
				Effekseer::Mat43f instMat = instanceParameter.SRTMatrix43;

				ApplyViewOffset(instMat, camera, instanceParameter.ViewOffsetDistance);
				
				CalcBillboard(parameter.Billboard, mat_rot, s, R, F, instMat, m_renderer->GetCameraFrontDirection());
			}
			else
			{
				CalcBillboard(parameter.Billboard, mat_rot, s, R, F, instanceParameter.SRTMatrix43, m_renderer->GetCameraFrontDirection());
			}
#else
			CalcBillboard(parameter.Billboard, mat_rot, s, R, F, instanceParameter.SRTMatrix43, m_renderer->GetCameraFrontDirection());
#endif

			for (int i = 0; i < 4; i++)
			{
				verteies[i].Pos.X = verteies[i].Pos.X * s.GetX();
				verteies[i].Pos.Y = verteies[i].Pos.Y * s.GetY();
			}

			ApplyDepthParameters(mat_rot,
								 m_renderer->GetCameraFrontDirection(),
								 m_renderer->GetCameraPosition(),
								 s,
								 parameter.DepthParameterPtr,
								 parameter.IsRightHand);

			TransformVertexes(verteies, 4, mat_rot);

			if (vertexType == VertexType::Dynamic || vertexType == VertexType::Lightning)
			{
				if (!parameter.IsRightHand)
				{
					F = -F;
				}

				StrideView<VERTEX> vs(verteies.pointerOrigin_, stride_, 4);
				for (auto i = 0; i < 4; i++)
				{
					vs[i].SetPackedNormal(PackVector3DF(F));
					vs[i].SetPackedTangent(PackVector3DF(R));
				}
			}
		}
		else if (parameter.Billboard == ::Effekseer::BillboardType::Fixed)
		{
			auto mat = instanceParameter.SRTMatrix43;

#ifdef __EFFEKSEER_BUILD_VERSION16__
			if (parameter.EnableViewOffset == true)
			{
				ApplyViewOffset(mat, camera, instanceParameter.ViewOffsetDistance);
			}
#endif

			ApplyDepthParameters(mat,
								 m_renderer->GetCameraFrontDirection(),
								 m_renderer->GetCameraPosition(),
								 parameter.DepthParameterPtr,
								 parameter.IsRightHand);

			for (int i = 0; i < 4; i++)
			{
				auto Pos = ::Effekseer::Vec3f::Load(&verteies[i].Pos);
				Pos = ::Effekseer::Vec3f::Transform(Pos, mat);
				::Effekseer::Vec3f::Store(&verteies[i].Pos, Pos);

				// distortion
				if (vertexType == VertexType::Distortion)
				{
					auto vs = (VERTEX_DISTORTION*)&verteies[i];

					::Effekseer::Vec3f t = mat.GetTranslation();

					auto Tangent = ::Effekseer::Vec3f::Load(&vs->Tangent);
					Tangent = ::Effekseer::Vec3f::Transform(Tangent, mat) - t;
					::Effekseer::Vec3f::Store(&vs->Tangent, Tangent);

					auto Binormal = ::Effekseer::Vec3f::Load(&vs->Binormal);
					Binormal = ::Effekseer::Vec3f::Transform(Binormal, mat) - t;
					::Effekseer::Vec3f::Store(&vs->Binormal, Binormal);
				}
				else if (vertexType == VertexType::Dynamic || vertexType == VertexType::Lightning)
				{
					StrideView<VERTEX> vs(verteies.pointerOrigin_, stride_, 4);
					auto tangentX = efkVector3D(mat.X.GetX(), mat.Y.GetX(), mat.Z.GetX());
					auto tangentZ = efkVector3D(mat.X.GetZ(), mat.Y.GetZ(), mat.Z.GetZ());
					tangentX = tangentX.Normalize();
					tangentZ = tangentZ.Normalize();

					if (!parameter.IsRightHand)
					{
						tangentZ = -tangentZ;
					}

					vs[i].SetPackedNormal(PackVector3DF(tangentZ));
					vs[i].SetPackedTangent(PackVector3DF(tangentX));
				}
			}
		}

		// custom parameter
		if (customData1Count_ > 0)
		{
			StrideView<float> custom(m_ringBufferData + sizeof(DynamicVertex), stride_, 4);
			for (int i = 0; i < 4; i++)
			{
				auto c = (float*)(&custom[i]);
				memcpy(c, instanceParameter.CustomData1.data(), sizeof(float) * customData1Count_);
			}
		}

		if (customData2Count_ > 0)
		{
			StrideView<float> custom(m_ringBufferData + sizeof(DynamicVertex) + sizeof(float) * customData1Count_, stride_, 4);
			for (int i = 0; i < 4; i++)
			{
				auto c = (float*)(&custom[i]);
				memcpy(c, instanceParameter.CustomData2.data(), sizeof(float) * customData2Count_);
			}
		}

		m_ringBufferData += (stride_ * 4);
		m_spriteCount++;
	}

	void EndRendering_(RENDERER* renderer, const efkSpriteNodeParam& param)
	{
		if (param.ZSort != Effekseer::ZSortType::None)
		{
			for (auto& kv : instances)
			{
				efkVector3D t = kv.Value.SRTMatrix43.GetTranslation();

				auto frontDirection = m_renderer->GetCameraFrontDirection();
				if (!param.IsRightHand)
				{
					frontDirection.Z = -frontDirection.Z;
				}

				kv.Key = Effekseer::Vec3f::Dot(t, frontDirection);
			}

			if (param.ZSort == Effekseer::ZSortType::NormalOrder)
			{
				std::sort(instances.begin(), instances.end(), [](const KeyValue& a, const KeyValue& b) -> bool { return a.Key < b.Key; });
			}
			else
			{
				std::sort(instances.begin(), instances.end(), [](const KeyValue& a, const KeyValue& b) -> bool { return a.Key > b.Key; });
			}

			for (auto& kv : instances)
			{
				auto camera = m_renderer->GetCameraMatrix();
				const auto& state = renderer->GetStandardRenderer()->GetState();

				RenderingInstance(kv.Value, param, state, camera);
			}
		}
	}

public:
	void BeginRendering(const efkSpriteNodeParam& parameter, int32_t count, void* userData) override
	{
		BeginRendering_(m_renderer, count, parameter);
	}

	void Rendering(const efkSpriteNodeParam& parameter, const efkSpriteInstanceParam& instanceParameter, void* userData) override
	{
		if (m_spriteCount == m_renderer->GetSquareMaxCount())
			return;
		Rendering_(parameter, instanceParameter, userData, m_renderer->GetCameraMatrix());
	}

	void EndRendering(const efkSpriteNodeParam& parameter, void* userData) override
	{
		EndRendering_(m_renderer, parameter);
	}
};
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace EffekseerRenderer
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif // __EFFEKSEERRENDERER_SPRITE_RENDERER_H__

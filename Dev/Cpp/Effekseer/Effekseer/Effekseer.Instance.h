﻿
#ifndef __EFFEKSEER_INSTANCE_H__
#define __EFFEKSEER_INSTANCE_H__

//----------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------
#include "Effekseer.Base.h"

#include "SIMD/Effekseer.Mat43f.h"
#include "SIMD/Effekseer.Mat44f.h"
#include "SIMD/Effekseer.Vec2f.h"
#include "SIMD/Effekseer.Vec3f.h"
#include "SIMD/Effekseer.Vec4f.h"

#include "Effekseer.Color.h"
#include "Effekseer.IntrusiveList.h"
#include "Effekseer.Matrix43.h"
#include "Effekseer.Random.h"
#include "Effekseer.RectF.h"

#include "Effekseer.EffectNodeModel.h"
#include "Effekseer.EffectNodeRibbon.h"
#include "Effekseer.EffectNodeRing.h"
#include "Effekseer.EffectNodeSprite.h"
#include "Effekseer.EffectNodeTrack.h"
#include "ForceField/ForceFields.h"

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace Effekseer
{

struct InstanceCustomData
{
	union
	{
		struct
		{
			Vec2f start;
			Vec2f end;
		} easing;

		struct
		{
			Vec2f value;
		} random;

		struct
		{
			Vec2f offset;
		} fcruve;

		struct
		{
			std::array<float, 4> offset;
		} fcurveColor;
	};
};

/**
	@brief	エフェクトの実体
*/
class alignas(16) Instance : public IntrusiveList<Instance>::Node
{
	friend class Manager;
	friend class InstanceContainer;

protected:
	//! custom data
	InstanceCustomData customDataValues1;
	InstanceCustomData customDataValues2;

	Vec3f prevPosition_;

#ifdef __EFFEKSEER_BUILD_VERSION16__
	Vec3f parentPosition_;
	Vec3f steeringVec_;
#endif

public:
	static const int32_t ChildrenMax = 16;

	// マネージャ
	Manager* m_pManager;

	// パラメーター
	EffectNodeImplemented* m_pEffectNode;

	// コンテナ
	InstanceContainer* m_pContainer;

	// a group which the instance belongs to
	// 自分が所属するグループ
	InstanceGroup* ownGroup_;

	// a head of list in children group
	// 子グループの連結リストの先頭
	InstanceGroup* childrenGroups_;

	// 親
	Instance* m_pParent;

	// Random generator
	RandObject m_randObject;

	// グローバル位置
	Vec3f m_GlobalPosition;
	Vec3f m_GlobalVelocity;

	// グローバル位置補正
	Vec3f m_GlobalRevisionLocation;
	Vec3f m_GlobalRevisionVelocity;

#ifdef OLD_LF
	//! for noise
	Vec3f modifyWithNoise_;
#else
	LocalForceFieldInstance localForceField_;
#endif
	// Color for binding
	Color ColorInheritance;

	// Parent color
	Color ColorParent;

#ifdef __EFFEKSEER_BUILD_VERSION16__
	struct
	{
		float maxFollowSpeed;
		float steeringSpeed;
	} followParentParam;
#endif

	union
	{
		struct
		{
			Vec3f location;
		} fixed;

		struct
		{
			Vec3f location;
			Vec3f velocity;
			Vec3f acceleration;
		} random;

		struct
		{
			Vec3f start;
			Vec3f end;
		} easing;

		struct
		{
			Vec3f offset;
		} fcruve;

#ifdef __EFFEKSEER_BUILD_VERSION16__
		struct
		{
			float distance;
		} view_offset;
#endif

	} translation_values;

	union
	{
		struct
		{
			Vec3f rotation;
		} fixed;

		struct
		{
			Vec3f rotation;
			Vec3f velocity;
			Vec3f acceleration;
		} random;

		struct
		{
			Vec3f start;
			Vec3f end;
		} easing;

		struct
		{
			float rotation;
			Vec3f axis;

			union
			{
				struct
				{
					float rotation;
					float velocity;
					float acceleration;
				} random;

				struct
				{
					float start;
					float end;
				} easing;
			};
		} axis;

		struct
		{
			Vec3f offset;
		} fcruve;

	} rotation_values;

	union
	{
		struct
		{
			Vec3f scale;
		} fixed;

		struct
		{
			Vec3f scale;
			Vec3f velocity;
			Vec3f acceleration;
		} random;

		struct
		{
			Vec3f start;
			Vec3f end;
		} easing;

		struct
		{
			float scale;
			float velocity;
			float acceleration;
		} single_random;

		struct
		{
			float start;
			float end;
		} single_easing;

		struct
		{
			Vec3f offset;
		} fcruve;

	} scaling_values;

	// 描画
	union
	{
		EffectNodeSprite::InstanceValues sprite;
		EffectNodeRibbon::InstanceValues ribbon;
		EffectNodeRing::InstanceValues ring;
		EffectNodeModel::InstanceValues model;
		EffectNodeTrack::InstanceValues track;
	} rendererValues;

	// 音
	union
	{
		int32_t delay;
	} soundValues;

	// 状態
	eInstanceState m_State;

	// 生存時間
	float m_LivedTime;

	// 生成されてからの時間
	float m_LivingTime;

#ifdef __EFFEKSEER_BUILD_VERSION16__
	//! The time offset for UV animation
	int32_t uvTimeOffsets[ParameterRendererCommon::UVParameterNum];

	// Scroll, FCurve area for UV
	RectF uvAreaOffsets[ParameterRendererCommon::UVParameterNum];

	// Scroll speed for UV
	Vec2f uvScrollSpeeds[ParameterRendererCommon::UVParameterNum];
#else
	//! The time offset for UV animation
	int32_t uvTimeOffset = 0;

	// Scroll, FCurve area for UV
	RectF uvAreaOffset;

	// Scroll speed for UV
	Vec2f uvScrollSpeed;
#endif

	// The number of generated chiledren. (fixed size)
	int32_t m_fixedGeneratedChildrenCount[ChildrenMax];

	// The number of maximum generated chiledren. (fixed size)
	int32_t fixedMaxGenerationChildrenCount_[ChildrenMax];

	// The time to generate next child.  (fixed size)
	float m_fixedNextGenerationTime[ChildrenMax];

	// The number of generated chiledren. (flexible size)
	int32_t* m_flexibleGeneratedChildrenCount;

	// The number of maximum generated chiledren. (flexible size)
	int32_t* flexibleMaxGenerationChildrenCount_ = nullptr;

	// The time to generate next child.  (flexible size)
	float* m_flexibleNextGenerationTime;

	// The number of generated chiledren. (actually used)
	int32_t* m_generatedChildrenCount;

	// The number of maximum generated chiledren. (actually used)
	int32_t* maxGenerationChildrenCount = nullptr;

	// The time to generate next child.  (actually used)
	float* m_nextGenerationTime;

	// Spawning Method matrix
	Mat43f m_GenerationLocation;

	// 変換用行列
	Mat43f m_GlobalMatrix43;

	// 親の変換用行列
	Mat43f m_ParentMatrix;

	// FirstUpdate実行前
	bool m_IsFirstTime;

	// 変換用行列が計算済かどうか
	bool m_GlobalMatrix43Calculated;

	// 親の変換用行列が計算済かどうか
	bool m_ParentMatrix43Calculated;

	//! whether a time is allowed to pass
	bool is_time_step_allowed;

	int32_t m_InstanceNumber;

	/* 更新番号 */
	uint32_t m_sequenceNumber;

#ifdef __EFFEKSEER_BUILD_VERSION16__
	float m_flipbookIndexAndNextRate;

	union
	{
		struct
		{
		} fixed;

		struct
		{
			float begin_threshold;
			int32_t transition_frame;
			float no2_threshold;
			float no3_threshold;
			int32_t transition_frame2;
			float end_threshold;
		} four_point_interpolation;

		struct
		{
			float start;
			float end;
		} easing;

		struct
		{
			float offset;
		} fcurve;

	} alpha_cutoff_values;

	float m_AlphaThreshold;
#endif

	//! calculate dynamic equation and assign a result
	template <typename T, typename U>
	void ApplyEq(T& dstParam, Effect* e, InstanceGlobal* instg, IRandObject* rand, int dpInd, const U& originalParam);

	//! calculate dynamic equation and return a result
	template <typename S>
	Vec3f ApplyEq(Effect* e,
				  InstanceGlobal* instg,
				  IRandObject* rand,
				  const int& dpInd,
				  const Vec3f& originalParam,
				  const S& scale,
				  const S& scaleInv);

	//! calculate dynamic equation and return a result
	random_float ApplyEq(Effect* e, InstanceGlobal* instg, IRandObject* rand, const RefMinMax& dpInd, random_float originalParam);

	//! calculate dynamic equation and return a result
	template <typename S>
	random_vector3d ApplyEq(Effect* e,
							InstanceGlobal* instg,
							IRandObject* rand,
							const RefMinMax& dpInd,
							random_vector3d originalParam,
							const S& scale,
							const S& scaleInv);

	//! calculate dynamic equation and return a result
	random_int ApplyEq(Effect* e, InstanceGlobal* instg, IRandObject* rand, const RefMinMax& dpInd, random_int originalParam);

	// コンストラクタ
	Instance(Manager* pManager, EffectNode* pEffectNode, InstanceContainer* pContainer, InstanceGroup* pGroup);

	// デストラクタ
	virtual ~Instance();

	void GenerateChildrenInRequired();

	void UpdateChildrenGroupMatrix();

	InstanceGlobal* GetInstanceGlobal();

public:
	bool IsFirstTime() const
	{
		return m_IsFirstTime;
	}

	/**
		@brief	状態の取得
	*/
	eInstanceState GetState() const;

	/**
		@brief	行列の取得
	*/
	const Mat43f& GetGlobalMatrix43() const;

	/**
		@brief	初期化
	*/
	void Initialize(Instance* parent, int32_t instanceNumber, const Mat43f& globalMatrix);

	/**
		@brief	初回の更新
	*/
	void FirstUpdate();

	/**
		@brief	更新
	*/
	void Update(float deltaFrame, bool shown);

	/**
		@brief	Draw instance
	*/
	void Draw(Instance* next);

	/**
		@brief	破棄
	*/
	void Kill();

	/**
		@brief	UVの位置取得
	*/
#ifdef __EFFEKSEER_BUILD_VERSION16__
	RectF GetUV(const int32_t index) const;
#else
	RectF GetUV() const;
#endif

	//! get custom data
	std::array<float, 4> GetCustomData(int32_t index) const;

	//! get random object
	RandObject& GetRandObject()
	{
		return m_randObject;
	}

private:
	/**
		@brief	行列の更新
	*/
	void CalculateMatrix(float deltaFrame);

	/**
		@brief	行列の更新
	*/
	void CalculateParentMatrix(float deltaFrame);

	/**
		@brief	絶対パラメータの反映
	*/
	void ModifyMatrixFromLocationAbs(float deltaFrame);

	void ApplyDynamicParameterToFixedLocation();

	void ApplyDynamicParameterToFixedRotation();

	void ApplyDynamicParameterToFixedScaling();
};

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
} // namespace Effekseer
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
#endif // __EFFEKSEER_INSTANCE_H__


cbuffer VS_ConstantBuffer : register(b0)
{
    float4x4 mCamera;
    float4x4 mProj;
    float4 mUVInversed;

    float4 mflipbookParameter; // x:enable, y:loopType, z:divideX, w:divideY
};

struct VS_Input
{
	float3 Pos		: POSITION0;
	float4 Color		: NORMAL0;
	float2 UV		: TEXCOORD0;

	float3 Binormal		: NORMAL1;
	float3 Tangent		: NORMAL2;
    
    float2 AlphaUV          : TEXCOORD1;
    float2 UVDistortionUV   : TEXCOORD2;
    float2 BlendUV          : TEXCOORD3;
    float2 BlendAlphaUV     : TEXCOORD4;
    float2 BlendUVDistortionUV : TEXCOORD5;
    float FlipbookIndex     : TEXCOORD6;
    float AlphaThreshold    : TEXCOORD7;
};

struct VS_Output
{
	float4 Position		: SV_POSITION;
	float4 Color		: COLOR;
	float2 UV		: TEXCOORD0;

	float4 Pos		: TEXCOORD1;
	float4 PosU		: TEXCOORD2;
	float4 PosR		: TEXCOORD3;
    
    float2 AlphaUV              : TEXCOORD4;
    float2 UVDistortionUV       : TEXCOORD5;
    float2 BlendUV              : TEXCOORD6;
    float2 BlendAlphaUV         : TEXCOORD7;
    float2 BlendUVDistortionUV  : TEXCOORD8;
    float FlipbookRate          : TEXCOORD9;
    float2 FlipbookNextIndexUV  : TEXCOORD10;
    float AlphaThreshold        : TEXCOORD11;
};

#include "FlipbookInterpolationUtils.fx"

VS_Output main( const VS_Input Input )
{
	VS_Output Output = (VS_Output)0;
	float4 pos4 = { Input.Pos.x, Input.Pos.y, Input.Pos.z, 1.0 };

	float4 localBinormal = { Input.Pos.x + Input.Binormal.x, Input.Pos.y + Input.Binormal.y, Input.Pos.z + Input.Binormal.z, 1.0 };
	float4 localTangent = { Input.Pos.x + Input.Tangent.x, Input.Pos.y + Input.Tangent.y, Input.Pos.z + Input.Tangent.z, 1.0 };
	localBinormal = mul(mCamera, localBinormal);
	localTangent = mul(mCamera, localTangent);

	float4 cameraPos = mul(mCamera, pos4);
		cameraPos = cameraPos / cameraPos.w;

	localBinormal = localBinormal / localBinormal.w;
	localTangent = localTangent / localTangent.w;

	localBinormal = cameraPos + normalize(localBinormal - cameraPos);
	localTangent = cameraPos + normalize(localTangent - cameraPos);


	Output.Position = mul(mProj, cameraPos);

	Output.Pos = Output.Position;

	Output.PosU = mul(mProj, localBinormal);
	Output.PosR = mul(mProj, localTangent);

	Output.PosU /= Output.PosU.w;
	Output.PosR /= Output.PosR.w;
	Output.Pos /= Output.Pos.w;

	Output.Color = Input.Color;
	Output.UV = Input.UV;

	Output.UV.y = mUVInversed.x + mUVInversed.y * Input.UV.y;
    
    // alpha uv
    Output.AlphaUV = Input.AlphaUV;
    Output.AlphaUV.y = mUVInversed.x + mUVInversed.y * Input.AlphaUV.y;
    
    // uv distortion uv
    Output.UVDistortionUV = Input.UVDistortionUV;
    Output.UVDistortionUV.y = mUVInversed.x + mUVInversed.y * Input.UVDistortionUV.y;
    
    // blend texture
    Output.BlendUV = Input.BlendUV;
    Output.BlendUV.y = mUVInversed.x + mUVInversed.y * Input.BlendUV.y;
    
    // blend alpha texture
    Output.BlendAlphaUV = Input.BlendAlphaUV;
    Output.BlendAlphaUV.y = mUVInversed.x + mUVInversed.y * Input.BlendAlphaUV.y;
    
    // blend uv distortion texture
    Output.BlendUVDistortionUV = Input.BlendUVDistortionUV;
    Output.BlendUVDistortionUV.y = mUVInversed.x + mUVInversed.y * Input.BlendUVDistortionUV.y;
    
    // flipbook interpolation
	ApplyFlipbookVS(Output.FlipbookRate, Output.FlipbookNextIndexUV, mflipbookParameter, Input.FlipbookIndex, Output.UV);
    
    // alpha threshold
    Output.AlphaThreshold = Input.AlphaThreshold;

	return Output;
}

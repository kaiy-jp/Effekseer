#include "EffekseerMaterialCompilerDX12.h"
#include <iostream>

#include "../3rdParty/LLGI/src/DX12/LLGI.CompilerDX12.h"
#include "../DirectX/ShaderGenerator.h"

#undef min

#pragma comment(lib, "d3dcompiler.lib")

namespace Effekseer
{

void Serialize(std::vector<uint8_t>& dst, const LLGI::CompilerResult& result)
{

	uint32_t binarySize = 0;

	binarySize += sizeof(uint32_t);

	for (size_t i = 0; i < result.Binary.size(); i++)
	{
		binarySize += sizeof(uint32_t);
		binarySize += result.Binary[i].size();
	}

	dst.resize(binarySize);

	uint32_t offset = 0;
	uint32_t count = result.Binary.size();

	memcpy(dst.data() + offset, &count, sizeof(int32_t));
	offset += sizeof(int32_t);

	for (size_t i = 0; i < result.Binary.size(); i++)
	{
		uint32_t size = result.Binary[i].size();

		memcpy(dst.data() + offset, &size, sizeof(int32_t));
		offset += sizeof(int32_t);

		memcpy(dst.data() + offset, result.Binary[i].data(), result.Binary[i].size());
		offset += result.Binary[i].size();
	}
}

namespace DX12
{

#define _DIRECTX12 1
#include "../HLSL/HLSL.h"

} // namespace DX12

} // namespace Effekseer

namespace Effekseer
{

class CompiledMaterialBinaryDX12 : public CompiledMaterialBinary, ReferenceObject
{
private:
	std::array<std::vector<uint8_t>, static_cast<int32_t>(MaterialShaderType::Max)> vertexShaders_;

	std::array<std::vector<uint8_t>, static_cast<int32_t>(MaterialShaderType::Max)> pixelShaders_;

public:
	CompiledMaterialBinaryDX12() {}

	virtual ~CompiledMaterialBinaryDX12() {}

	void SetVertexShaderData(MaterialShaderType type, const std::vector<uint8_t>& data)
	{
		vertexShaders_.at(static_cast<int>(type)) = data;
	}

	void SetPixelShaderData(MaterialShaderType type, const std::vector<uint8_t>& data) { pixelShaders_.at(static_cast<int>(type)) = data; }

	const uint8_t* GetVertexShaderData(MaterialShaderType type) const override { return vertexShaders_.at(static_cast<int>(type)).data(); }

	int32_t GetVertexShaderSize(MaterialShaderType type) const override { return vertexShaders_.at(static_cast<int>(type)).size(); }

	const uint8_t* GetPixelShaderData(MaterialShaderType type) const override { return pixelShaders_.at(static_cast<int>(type)).data(); }

	int32_t GetPixelShaderSize(MaterialShaderType type) const override { return pixelShaders_.at(static_cast<int>(type)).size(); }

	int AddRef() override { return ReferenceObject::AddRef(); }

	int Release() override { return ReferenceObject::Release(); }

	int GetRef() override { return ReferenceObject::GetRef(); }
};

CompiledMaterialBinary* MaterialCompilerDX12::Compile(Material* material, int32_t maximumTextureCount)
{
	// to use options
	// auto compiler = LLGI::CreateSharedPtr(LLGI::CreateCompiler(LLGI::DeviceType::DirectX12));
	auto compiler = LLGI::CreateSharedPtr(new LLGI::CompilerDX12(LLGI::CompilerDX12Option::ColumnMajor));

	auto binary = new CompiledMaterialBinaryDX12();

	auto convertToVectorVS = [compiler](const std::string& str) -> std::vector<uint8_t> {
		std::vector<uint8_t> ret;

		LLGI::CompilerResult result;
		compiler->Compile(result, str.c_str(), LLGI::ShaderStageType::Vertex);

		if (result.Binary.size() > 0)
		{
			Serialize(ret, result);
		}
		else
		{
			std::cout << "VertexShader Compile Error" << std::endl;
			std::cout << result.Message << std::endl;
			std::cout << str << std::endl;
		}

		return ret;
	};

	auto convertToVectorPS = [compiler](const std::string& str) -> std::vector<uint8_t> {
		std::vector<uint8_t> ret;

		LLGI::CompilerResult result;
		compiler->Compile(result, str.c_str(), LLGI::ShaderStageType::Pixel);

		if (result.Binary.size() > 0)
		{
			Serialize(ret, result);
		}
		else
		{
			std::cout << "PixelShader Compile Error" << std::endl;
			std::cout << result.Message << std::endl;
			std::cout << str << std::endl;
		}

		return ret;
	};

	auto saveBinary = [&material, &binary, &convertToVectorVS, &convertToVectorPS, &maximumTextureCount](MaterialShaderType type) {
		auto generator = DirectX::ShaderGenerator(DX12::material_common_define,
												  DX12::material_common_vs_functions,
												  DX12::material_sprite_vs_pre,
												  DX12::material_sprite_vs_pre_simple,
												  DX12::model_vs_pre,
												  DX12::material_sprite_vs_suf1,
												  DX12::material_sprite_vs_suf1_simple,
												  DX12::model_vs_suf1,
												  DX12::material_sprite_vs_suf2,
												  DX12::model_vs_suf2,
												  DX12::g_material_ps_pre,
												  DX12::g_material_ps_suf1,
												  DX12::g_material_ps_suf2_lit,
												  DX12::g_material_ps_suf2_unlit,
												  DX12::g_material_ps_suf2_refraction);

		auto shader = generator.GenerateShader(material, type, maximumTextureCount, 8, 1);
		binary->SetVertexShaderData(type, convertToVectorVS(shader.CodeVS));
		binary->SetPixelShaderData(type, convertToVectorPS(shader.CodePS));
	};

	if (material->GetHasRefraction())
	{
		saveBinary(MaterialShaderType::Refraction);
		saveBinary(MaterialShaderType::RefractionModel);
	}

	saveBinary(MaterialShaderType::Standard);
	saveBinary(MaterialShaderType::Model);

	return binary;
}

CompiledMaterialBinary* MaterialCompilerDX12::Compile(Material* material) { return Compile(material, Effekseer::UserTextureSlotMax); }

} // namespace Effekseer

#ifdef __SHARED_OBJECT__

extern "C"
{

#ifdef _WIN32
#define EFK_EXPORT __declspec(dllexport)
#else
#define EFK_EXPORT
#endif

	EFK_EXPORT Effekseer::MaterialCompiler* EFK_STDCALL CreateCompiler() { return new Effekseer::MaterialCompilerDX12(); }
}
#endif
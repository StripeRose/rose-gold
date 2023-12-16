// Filter "Resources"

#pragma once

#include "DX12_ComPtr.hpp"
#include "DX12_Shader.hpp"

#include <Graphics_Mesh.hpp>

#include <d3d12.h>

namespace RoseGold::DirectX12
{
	class Manager;
	class Mesh : public Core::Graphics::Mesh
	{
	public:
		static void Prepare(ID3D12Device* aDevice);
		static ID3D12RootSignature* GetRootSignature();
		static void Cleanup();

	public:
		Mesh(Manager& aManager);
		~Mesh() = default;

	private:
		static void CreateRootSignature(ID3D12Device* aDevice);
		static void CreateShaders();

		static ComPtr<ID3D12RootSignature> ourMeshRootSignature;
		static std::shared_ptr<Shader> ourVertexShader;
		static std::shared_ptr<Shader> ourPixelShader;
	};
}

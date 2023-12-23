// Filter "Resources"

#pragma once

#include "DX12_ComPtr.hpp"
#include "DX12_Device.hpp"

#include <Graphics_Pipeline.hpp>

#include <map>
#include <memory>

namespace RoseGold::DirectX12
{
	class CachedPipelineState : public Core::Graphics::CachedPipelineState
	{
	public:
		ComPtr<ID3D12PipelineState> myPipelineState;
	};

	class Pipeline
	{
	public:
		Pipeline(Device& aDevice);

		std::shared_ptr<CachedPipelineState> CreateOrGetState(const Core::Graphics::PipelineState& aPipelineState);
		
	private:
		void SetupRootSignature();

		Device& myDevice;

		ComPtr<ID3D12RootSignature> myRootSignature;
	};
}
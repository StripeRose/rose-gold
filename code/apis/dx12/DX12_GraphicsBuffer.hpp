// Filter "Resources"

#pragma once

#include "DX12_ComPtr.hpp"

#include <Graphics_Buffer.hpp>

#include <d3d12.h>

namespace RoseGold::DirectX12
{
	class GPUResource
	{
	public:
		inline GPUResource(ComPtr<ID3D12Resource> aResource, D3D12_RESOURCE_STATES aUsageState)
			: myResource(aResource)
			, myUsageState(aUsageState)
			, myGPUAddress(aResource->GetGPUVirtualAddress())
			, myIsReady(false)
		{ }
		virtual ~GPUResource() = default;

		ComPtr<ID3D12Resource> GetResource() { return myResource; }
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return myGPUAddress; }
		D3D12_RESOURCE_STATES GetUsageState() const { return myUsageState; }

		bool IsReady() const { return myIsReady; }

	protected:
		ComPtr<ID3D12Resource> myResource;
		D3D12_GPU_VIRTUAL_ADDRESS myGPUAddress;
		D3D12_RESOURCE_STATES myUsageState;
		bool myIsReady;
	};

	class Device;
	class NativeGraphicsBuffer : public Core::Graphics::GraphicsBuffer, public GPUResource
	{
	public:
		NativeGraphicsBuffer(ComPtr<ID3D12Resource> aResource, D3D12_RESOURCE_STATES aUsageState);

		// Implementing GraphicsBuffer
	public:
		void* GetNativeBufferPtr() override { return myResource.Get(); }

	protected:
		std::uint32_t Align(std::uint32_t aLocation, std::uint32_t anAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	};

	class NativeVertexBuffer : public NativeGraphicsBuffer
	{
	public:
		NativeVertexBuffer(Device& aDevice, std::uint32_t aVertexCount, std::uint32_t aVertexStride);

		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return myBufferView; }

		// Implementing GraphicsBuffer
	public:
		void SetData(const void* aDataPtr, std::uint32_t aDataSize) override;

		std::uint32_t GetCount() const override { return myBufferView.SizeInBytes / myBufferView.StrideInBytes; }
		std::uint32_t GetStride() const override { return myBufferView.StrideInBytes; }
		Target GetTarget() const override { return Target::Vertex; }

	private:
		ComPtr<ID3D12Resource> CreateResource(Device& aDevice, std::uint32_t aVertexCount, std::uint32_t aVertexStride);

	private:
		D3D12_VERTEX_BUFFER_VIEW myBufferView;
	};

	class NativeConstantBuffer : public NativeGraphicsBuffer
	{
	public:
		NativeConstantBuffer(Device& aDevice, std::uint32_t aBufferSize);
		virtual ~NativeConstantBuffer() override;

		void SetData(const void* aDataPtr, std::uint32_t aDataSize) override;
		const DescriptorHeapHandle& GetViewHandle() const { return *myConstantBufferViewHandle; }

	private:
		ComPtr<ID3D12Resource> CreateResource(Device& aDevice, std::uint32_t aBufferSize);

	private:
		void* myMappedBuffer;
		std::uint32_t myBufferSize;

		std::shared_ptr<DescriptorHeapHandle> myConstantBufferViewHandle;
	};
}

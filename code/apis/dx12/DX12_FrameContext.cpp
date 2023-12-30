// Filter "Frame contexts"

#include "DX12_Device.hpp"
#include "DX12_Diagnostics.hpp"
#include "DX12_FrameContext.hpp"

namespace RoseGold::DirectX12
{
	FrameContext::FrameContext(Device& aDevice, D3D12_COMMAND_LIST_TYPE aCommandType)
		: myDevice(aDevice)
		, myCommandType(aCommandType)
		, myCurrentFrameHeap(nullptr)
		, myResourceBarrierQueue()
	{
		AssertSuccess(
			aDevice.GetDevice()->CreateCommandAllocator(
				aCommandType,
				IID_PPV_ARGS(myCommandAllocator.ReleaseAndGetAddressOf())
			)
		);

		AssertSuccess(
			aDevice.GetDevice()->CreateCommandList(
				0,
				aCommandType,
				myCommandAllocator.Get(),
				nullptr,
				IID_PPV_ARGS(myCommandList.ReleaseAndGetAddressOf()))
		);

		if (myCommandType != D3D12_COMMAND_LIST_TYPE_COPY)
			BindDescriptorHeaps();
	}

	void FrameContext::Reset()
	{
		myCommandAllocator->Reset();
		myCommandList->Reset(myCommandAllocator.Get(), nullptr);
		myFrameConstantBuffers.clear();

		if (myCommandType != D3D12_COMMAND_LIST_TYPE_COPY)
			BindDescriptorHeaps();
	}

	void FrameContext::AddBarrier(GPUResource& aResource, D3D12_RESOURCE_STATES aNewState)
	{
		if (myQueuedBarriers >= ourMaxQueuedBarriers)
			FlushBarriers();

		if (myCommandType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			constexpr D3D12_RESOURCE_STATES VALID_COMPUTE_CONTEXT_STATES = (D3D12_RESOURCE_STATE_UNORDERED_ACCESS | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE);

			const D3D12_RESOURCE_STATES oldState = aResource.GetUsageState();
			Debug::Assert((oldState & VALID_COMPUTE_CONTEXT_STATES) == oldState, "Old resource state not valid for compute queues.");
			Debug::Assert((aNewState & VALID_COMPUTE_CONTEXT_STATES) == aNewState, "New resource state not valid for compute queues.");
		}

		std::optional<D3D12_RESOURCE_BARRIER> barrier = aResource.UpdateUsageState(aNewState);
		if (barrier.has_value())
		{
			myResourceBarrierQueue.at(myQueuedBarriers) = barrier.value();
			myQueuedBarriers++;
		}
	}

	void FrameContext::FlushBarriers()
	{
		if (myQueuedBarriers == 0)
			return;

		myCommandList->ResourceBarrier(static_cast<UINT>(myQueuedBarriers), myResourceBarrierQueue.data());
		myQueuedBarriers = 0;
	}

	void FrameContext::CopyResource(const GPUResource& aSource, GPUResource& aDestination)
	{
		myCommandList->CopyResource(aDestination.GetResource().Get(), aSource.GetResource().Get());
	}

	void FrameContext::CopyBufferRegion(const GPUResource& aSource, std::size_t aSourceOffset, GPUResource& aDestination, std::size_t aDestinationOffset, std::size_t aByteCountToCopy)
	{
		myCommandList->CopyBufferRegion(
			aDestination.GetResource().Get(),
			static_cast<UINT64>(aDestinationOffset),
			aSource.GetResource().Get(),
			static_cast<UINT64>(aSourceOffset),
			static_cast<UINT64>(aByteCountToCopy)
		);
	}

	void FrameContext::SetConstantBufferData(void* someData, std::size_t aDataSize)
	{
		const std::uint32_t dataSize = static_cast<std::uint32_t>(aDataSize);
		ConstantBuffer& cpuBuffer = *myFrameConstantBuffers.emplace_back(
			std::make_shared<ConstantBuffer>(
				myDevice,
				dataSize
			)
		);

		cpuBuffer.SetData(someData, dataSize);

		RenderPassDescriptorHeap& renderPassHeap = myDevice.GetDescriptorHeapManager().GetFrameHeap();
		std::shared_ptr<DescriptorHeapHandle> gpuBuffer = renderPassHeap.GetHeapHandleBlock(1);

		myDevice.GetDevice()->CopyDescriptorsSimple(
			1,
			gpuBuffer->GetCPUHandle(),
			cpuBuffer.GetViewHandle().GetCPUHandle(),
			renderPassHeap.GetHeapType()
		);

		myCommandList->SetGraphicsRootDescriptorTable(0, gpuBuffer->GetGPUHandle());
	}

	void FrameContext::BindDescriptorHeaps()
	{
		DescriptorHeapManager& heapManager = myDevice.GetDescriptorHeapManager();

		myCurrentFrameHeap = &heapManager.GetFrameHeap();
		myCurrentFrameHeap->Reset();

		ID3D12DescriptorHeap* heapsToBind[1];
		heapsToBind[0] = myCurrentFrameHeap->GetHeap().Get();
		//heapsToBind[1] = heapManager.GetFrameSamplerHeap().GetHeap().Get();

		myCommandList->SetDescriptorHeaps(1, heapsToBind);
	}

	FrameGraphicsContext::FrameGraphicsContext(Device& aDevice)
		: FrameContext(aDevice, D3D12_COMMAND_LIST_TYPE_DIRECT)
	{ }

	void FrameGraphicsContext::ClearColor(const Core::Graphics::RenderTexture& aTarget, Color aClearColor)
	{
		const RenderTarget& target = static_cast<const RenderTarget&>(aTarget);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = target.GetColorView()->GetCPUHandle();

		if (!Debug::Verify(handle.ptr != NULL, "Invalid target."))
			return;

		float color[4] = {
			aClearColor.R,
			aClearColor.G,
			aClearColor.B,
			aClearColor.A
		};
		myCommandList->ClearRenderTargetView(
			handle,
			color,
			0,
			nullptr
		);
	}

	void FrameGraphicsContext::ClearDepth(const Core::Graphics::RenderTexture& aTarget, float aDepth, std::uint8_t aStencil)
	{
		const RenderTarget& target = static_cast<const RenderTarget&>(aTarget);
		D3D12_CPU_DESCRIPTOR_HANDLE handle = target.GetDepthStencilView()->GetCPUHandle();

		if (!Debug::Verify(handle.ptr != NULL, "Invalid target."))
			return;

		myCommandList->ClearDepthStencilView(
			handle,
			D3D12_CLEAR_FLAG_DEPTH,
			aDepth,
			aStencil, 0, nullptr
		);
	}

	void FrameGraphicsContext::DisableScissorRect()
	{
		myCommandList->RSSetScissorRects(0, nullptr);
	}

	void FrameGraphicsContext::Dispatch(std::uint32_t aGroupCountX, std::uint32_t aGroupCountY, std::uint32_t aGroupCountZ)
	{
		myCommandList->Dispatch(aGroupCountX, aGroupCountY, aGroupCountZ);
	}

	void FrameGraphicsContext::Dispatch1D(std::uint32_t aThreadCountX, std::uint32_t aGroupSizeX)
	{
		Dispatch(GetGroupCount(aThreadCountX, aGroupSizeX), 1, 1);
	}

	void FrameGraphicsContext::Dispatch2D(std::uint32_t aThreadCountX, std::uint32_t aThreadCountY, std::uint32_t aGroupSizeX, std::uint32_t aGroupSizeY)
	{
		Dispatch(GetGroupCount(aThreadCountX, aGroupSizeX), GetGroupCount(aThreadCountY, aGroupSizeY), 1);
	}

	void FrameGraphicsContext::Dispatch3D(std::uint32_t aThreadCountX, std::uint32_t aThreadCountY, std::uint32_t aThreadCountZ, std::uint32_t aGroupSizeX, std::uint32_t aGroupSizeY, std::uint32_t aGroupSizeZ)
	{
		Dispatch(GetGroupCount(aThreadCountX, aGroupSizeX), GetGroupCount(aThreadCountY, aGroupSizeY), GetGroupCount(aThreadCountZ, aGroupSizeZ));
	}

	void FrameGraphicsContext::Draw(std::uint32_t aVertexCount, std::uint32_t aVertexStartOffset)
	{
		DrawInstanced(aVertexCount, 1, aVertexStartOffset, 0);
	}

	void FrameGraphicsContext::DrawIndexed(std::uint32_t anIndexCount, std::uint32_t aStartIndexLocation, std::uint32_t aBaseVertexLocation)
	{
		DrawIndexedInstanced(anIndexCount, 1, aStartIndexLocation, aBaseVertexLocation, 0);
	}

	void FrameGraphicsContext::DrawInstanced(std::uint32_t aVertexCountPerInstance, std::uint32_t anInstanceCount, std::uint32_t aStartVertexLocation, std::uint32_t aStartInstanceLocation)
	{
		myCommandList->DrawInstanced(aVertexCountPerInstance, anInstanceCount, aStartVertexLocation, aStartInstanceLocation);
	}

	void FrameGraphicsContext::DrawIndexedInstanced(std::uint32_t anIndexCountPerInstance, std::uint32_t anInstanceCount, std::uint32_t aStartIndexLocation, std::uint32_t aBaseVertexLocation, std::uint32_t aStartInstanceLocation)
	{
		myCommandList->DrawIndexedInstanced(anIndexCountPerInstance, anInstanceCount, aStartIndexLocation, aBaseVertexLocation, aStartInstanceLocation);
	}

	void FrameGraphicsContext::SetBlendFactor(Color aBlendFactor)
	{
		float color[4] = { aBlendFactor.R, aBlendFactor.G, aBlendFactor.B, aBlendFactor.A };
		myCommandList->OMSetBlendFactor(color);
	}

	void FrameGraphicsContext::SetScissorRect(const Math::RectangleT<int>& aRectangle)
	{
		const Math::Vector2T<int> halfSize = (aRectangle.Size / 2);

		D3D12_RECT rect;
		rect.left = aRectangle.Center.X - halfSize.X;
		rect.right = aRectangle.Center.X + halfSize.X;
		rect.top = aRectangle.Center.Y - halfSize.Y;
		rect.bottom = aRectangle.Center.Y + halfSize.Y;

		myCommandList->RSSetScissorRects(1, &rect);
	}

	void FrameGraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY aTopology)
	{
		myCommandList->IASetPrimitiveTopology(aTopology);
	}

	void FrameGraphicsContext::SetPipelineState(CachedPipelineState& aPipelineState)
	{
		myCommandList->SetPipelineState(aPipelineState.PipelineState.Get());
		myCommandList->SetGraphicsRootSignature(myDevice.GetPipeline().GetRootSignature());

		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> colorHandles;

		for (std::size_t i = 0; i < aPipelineState.Outputs.size(); ++i)
		{
			RenderTarget* target = static_cast<RenderTarget*>(aPipelineState.Outputs.at(i).get());
			colorHandles.at(i) = target->GetColorView()->GetCPUHandle();
		}

		RenderTarget* depthTarget = static_cast<RenderTarget*>(aPipelineState.DepthTarget.get());
		const D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle = depthTarget->GetDepthStencilView()->GetCPUHandle();

		myCommandList->OMSetRenderTargets(
			static_cast<UINT>(aPipelineState.Outputs.size()),
			colorHandles.data(),
			false,
			&depthStencilHandle);
	}

	void FrameGraphicsContext::SetStencilRef(std::uint32_t aStencilRef)
	{
		myCommandList->OMSetStencilRef(aStencilRef);
	}

	void FrameGraphicsContext::SetVertexBuffer(unsigned int aStartSlot, const VertexBuffer& aBuffer)
	{
		myCommandList->IASetVertexBuffers(aStartSlot, 1, &aBuffer.GetBufferView());
	}

	void FrameGraphicsContext::SetViewportAndScissorRect(const Size& aScreenSize)
	{
		D3D12_VIEWPORT viewport;
		viewport.Width = static_cast<float>(aScreenSize.X);
		viewport.Height = static_cast<float>(aScreenSize.Y);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		D3D12_RECT scissor;
		scissor.top = 0;
		scissor.left = 0;
		scissor.bottom = aScreenSize.Y;
		scissor.right = aScreenSize.X;

		myCommandList->RSSetViewports(1, &viewport);
		myCommandList->RSSetScissorRects(1, &scissor);
	}

	void FrameGraphicsContext::SetViewport(const Math::Rectangle& aRectangle)
	{
		const Math::Vector2 topLeft = aRectangle.Center - (aRectangle.Size * 0.5f);

		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = topLeft.X;
		viewport.TopLeftY = topLeft.Y;
		viewport.Width = aRectangle.Size.X;
		viewport.Height = aRectangle.Size.Y;
		viewport.MinDepth = 0.1f;
		viewport.MaxDepth = 1.0f;

		myCommandList->RSSetViewports(1, &viewport);
	}
}
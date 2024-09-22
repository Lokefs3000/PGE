#pragma once

namespace pge
{
	struct RenderDeviceCreateInfo
	{
		bool DebuggingEnabled;
	};

	class SwapChain;
	struct SwapChainCreateInfo;
	class CommandList;
	struct CommandListCreateInfo;
	class DeviceBuffer;
	struct DeviceBufferCreateInfo;
	class GraphicsPipeline;
	struct GraphicsPipelineCreateInfo;

	class BaseRenderDevice
	{
	private:

	public:
		BaseRenderDevice();
		virtual ~BaseRenderDevice();

		virtual std::unique_ptr<SwapChain> CreateSwapChain(SwapChainCreateInfo&& createInfo) = 0;
		virtual std::unique_ptr<CommandList> CreateCommandList(CommandListCreateInfo&& createInfo) = 0;
		virtual std::unique_ptr<DeviceBuffer> CreateDeviceBuffer(DeviceBufferCreateInfo&& createInfo) = 0;
		virtual std::unique_ptr<GraphicsPipeline> CreateGraphicsPipeline(GraphicsPipelineCreateInfo&& createInfo) = 0;

		virtual void SubmitCommandList(CommandList* commandList) = 0;
		virtual void PresentSwapChain(SwapChain* swapChain) = 0;
	};
}
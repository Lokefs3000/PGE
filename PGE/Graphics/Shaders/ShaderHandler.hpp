#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include "Types/ThreadedQueue.hpp"

#include "Resources/ResourcePtr.hpp"

namespace tf { struct Executor; }

namespace pge
{
	class ShaderPackage;
	class ShaderResource;
	class BaseRenderDevice;

	struct ShaderQueueElement
	{
		ShaderPackage* Package;
		ResourcePtr<ShaderResource> Resource;
	};

	class ShaderHandler
	{
	private:
		std::vector<std::unique_ptr<ShaderPackage>> m_Packages;
		std::vector<std::pair<size_t, ResourcePtr<ShaderResource>>> m_Resources;

		ThreadedQueue<ShaderQueueElement> m_ShaderQueue;
		void LoadAndPopShader();

		BaseRenderDevice* m_RenderDevice = nullptr;
		tf::Executor* m_Executor = nullptr;
	public:
		ShaderHandler();
		~ShaderHandler();

		void LowMemory();

		void FlushShaders();
		void AddPackage(std::unique_ptr<ShaderPackage>& package); //gets moved

		ResourcePtr<ShaderResource> LoadResource(std::string_view name);

		void Connect(BaseRenderDevice* rd, tf::Executor* ex);
	};
}
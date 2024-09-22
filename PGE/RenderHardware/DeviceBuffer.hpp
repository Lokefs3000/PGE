#pragma once

#include <cstdint>

namespace pge
{
	enum class BufferUsage
	{
		Vertex,
		Index,
		Constant,
		Staging,
		TransferDst,
		TransferSrc
	};

	enum class BufferAccess
	{
		None,
		Read,
		Write,
		ReadWrite
	};

	struct DeviceBufferCreateInfo
	{
		BufferUsage Usage;
		BufferAccess Access;

		bool Dynamic;

		size_t Size;
		uint8_t Stride;
	};

	struct BufferRange
	{
		size_t Begin;
		size_t End;
	};

	class DeviceBuffer
	{
	private:
		
	public:
		DeviceBuffer();
		virtual ~DeviceBuffer();

		virtual void* Map() = 0;
		virtual void Unmap(BufferRange&& written) = 0;

		virtual size_t GetSize() = 0;
		virtual uint8_t GetStride() = 0;
		virtual size_t GetByteSize() = 0;
	};
}
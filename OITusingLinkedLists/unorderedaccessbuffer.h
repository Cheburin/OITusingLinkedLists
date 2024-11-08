#pragma once
#include "uniformbuffer.h"

namespace framework
{

	class UnorderedAccessBuffer : public UniformBuffer
	{
	public:
		UnorderedAccessBuffer();
		virtual ~UnorderedAccessBuffer();

		static D3D11_BUFFER_DESC getDefaultUnorderedAcces(unsigned int size, unsigned int structsize);

		bool initDefaultUnorderedAccess(ID3D11Device* device, size_t count, size_t structSize, unsigned int flags = 0)
		{
			D3D11_BUFFER_DESC desc = getDefaultUnorderedAcces(count, structSize);
			return initUnorderedAccess(device, count, structSize, desc, flags);
		}

		bool initUnorderedAccess(ID3D11Device* device, size_t count, size_t structSize, const D3D11_BUFFER_DESC& desc, unsigned int flags = 0);

		unsigned int getsetUnorderedAccessViewFlags() const
		{
			return m_uavFlags;
		}

		unsigned int getActualSize(ID3D11DeviceContext* context);

	protected:
		virtual void destroy();

	private:
		unsigned int m_uavFlags;
		ID3D11Buffer* m_stagingBuffer;
	};

}
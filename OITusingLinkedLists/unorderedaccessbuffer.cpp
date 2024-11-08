#include "DXUT.h"

#include <map>
#include <algorithm>
#include <vector>
#include <array>
#include <memory>
#include <assert.h>
#include <malloc.h>
#include <Exception>

#include "unorderedaccessbuffer.h"
#include "resourceview.h"
namespace framework
{

	D3D11_BUFFER_DESC UnorderedAccessBuffer::getDefaultUnorderedAcces(unsigned int size, unsigned int structsize)
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = size * structsize;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = structsize;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		return desc;
	}

	UnorderedAccessBuffer::UnorderedAccessBuffer() :
		m_uavFlags(0),
		m_stagingBuffer(0)
	{
		m_checkSizeOnSet = false;
	}

	UnorderedAccessBuffer::~UnorderedAccessBuffer()
	{
		destroy();
	}

	void UnorderedAccessBuffer::destroy()
	{
		if (m_stagingBuffer != 0)
		{
			m_stagingBuffer->Release();
			m_stagingBuffer = 0;
		}

		UniformBuffer::destroy();
	}

	bool UnorderedAccessBuffer::initUnorderedAccess(ID3D11Device* device, size_t count, size_t structSize, const D3D11_BUFFER_DESC& desc, unsigned int flags)
	{
		destroy();
		m_desc = desc;
		m_uavFlags = flags;

		HRESULT hr = device->CreateBuffer(&m_desc, 0, &m_buffer);
		if (hr != S_OK)
		{
			//utils::Logger::toLog("Error: could not create an unordered access buffer.\n");
			return false;
		}

		if (isStructured())
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC svdesc = ResourceView::getDefaultShaderDesc();
			svdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			svdesc.Buffer.FirstElement = 0;
			svdesc.Buffer.NumElements = count;
			m_view.setShaderDesc(svdesc);

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavdesc = ResourceView::getDefaultUAVDesc();
			uavdesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavdesc.Buffer.FirstElement = 0;
			uavdesc.Buffer.Flags = m_uavFlags;
			uavdesc.Buffer.NumElements = count;
			m_view.setUnorderedAccessDesc(uavdesc);

			m_view.init(device, m_buffer, m_desc.BindFlags);
		}

		D3D11_BUFFER_DESC stagingDesc;
		stagingDesc.ByteWidth = 4;
		stagingDesc.BindFlags = 0;
		stagingDesc.MiscFlags = 0;
		stagingDesc.Usage = D3D11_USAGE_STAGING;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		hr = device->CreateBuffer(&stagingDesc, nullptr, &m_stagingBuffer);
		if (hr != S_OK)
		{
			destroy();
			//utils::Logger::toLog("Error: could not create an unordered access buffer, staging buffer creation failed.\n");
			return false;
		}

		//if (m_buffer != 0) initDestroyable();
		return m_buffer != 0;
	}

	unsigned int UnorderedAccessBuffer::getActualSize(ID3D11DeviceContext* context)
	{
		context->CopyStructureCount(m_stagingBuffer, 0, m_view.asUAView());
		D3D11_MAPPED_SUBRESOURCE subresource;
		context->Map(m_stagingBuffer, 0, D3D11_MAP_READ, 0, &subresource);
		unsigned int numActiveElements = *(unsigned int*)subresource.pData;
		context->Unmap(m_stagingBuffer, 0);

		return numActiveElements;
	}

}
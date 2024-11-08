#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

extern SceneState scene_state;

extern CDXUTTextHelper*                    g_pTxtHelper;
extern CDXUTDialogResourceManager          g_DialogResourceManager;

SwapChainGraphicResources * SCG;
//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* device, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* backBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	g_DialogResourceManager.OnD3D11ResizedSwapChain(device, backBufferSurfaceDesc);

	float ratio = backBufferSurfaceDesc->Width / (FLOAT)backBufferSurfaceDesc->Height;
	
	float np = 0.1, fp = 1000, fov = D3DX_PI / 3;
	
	DirectX::XMStoreFloat4x4(&scene_state.mProjection, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fov, ratio, np, fp)));

	scene_state.vFrustumParams = SimpleMath::Vector4(backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height, scene_state.mProjection._22, ratio);

	scene_state.vFrustumNearFar = SimpleMath::Vector4(np, fp, 0, 0);

	SCG = new SwapChainGraphicResources();

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = backBufferSurfaceDesc->Width;
	textureDesc.Height = backBufferSurfaceDesc->Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;// DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 4;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = 0;
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;//DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
	sr_desc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

	SCG->depthStencilT.resize(2);
	SCG->depthStencilV.resize(2);
	SCG->depthStencilSRV.resize(2);

	hr = device->CreateTexture2D(&textureDesc, 0, SCG->depthStencilT[0].ReleaseAndGetAddressOf());
	hr = device->CreateTexture2D(&textureDesc, 0, SCG->depthStencilT[1].ReleaseAndGetAddressOf());

	hr = device->CreateDepthStencilView(SCG->depthStencilT[0].Get(), &dsv_desc, SCG->depthStencilV[0].ReleaseAndGetAddressOf());
	hr = device->CreateDepthStencilView(SCG->depthStencilT[1].Get(), &dsv_desc, SCG->depthStencilV[1].ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->depthStencilT[0].Get(), &sr_desc, SCG->depthStencilSRV[0].ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(SCG->depthStencilT[1].Get(), &sr_desc, SCG->depthStencilSRV[1].ReleaseAndGetAddressOf());

	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	SCG->colorLayerT.resize(4);
	SCG->colorLayerV.resize(4);
	SCG->colorLayerSRV.resize(4);

	for (int i = 0; i < 4; i++){

		hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->colorLayerT[i].ReleaseAndGetAddressOf());

		hr = device->CreateRenderTargetView(SCG->colorLayerT[i].Get(), nullptr, SCG->colorLayerV[i].ReleaseAndGetAddressOf());

		hr = device->CreateShaderResourceView(SCG->colorLayerT[i].Get(), nullptr, SCG->colorLayerSRV[i].ReleaseAndGetAddressOf());
	}

	/////
	textureDesc.Format = DXGI_FORMAT_R32_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	hr = device->CreateTexture2D(&textureDesc, 0, SCG->headBufferT.ReleaseAndGetAddressOf());
	hr = device->CreateShaderResourceView(SCG->headBufferT.Get(), 0, SCG->headBufferSRV.ReleaseAndGetAddressOf());
	hr = device->CreateUnorderedAccessView(SCG->headBufferT.Get(), 0, SCG->headBufferUAV.ReleaseAndGetAddressOf());
	/////

	//SCG->fragmentsBuffer = std::make_unique<framework::UnorderedAccessBuffer>();
	//auto m_fragmentsBufferSize = backBufferSurfaceDesc->Width *  backBufferSurfaceDesc->Height * 32;
	//unsigned int fragmentSize = 4 + 4 + 4; // color + depth + next
	//unsigned int fragmentsBufferFlags = D3D11_BUFFER_UAV_FLAG::D3D11_BUFFER_UAV_FLAG_COUNTER;// D3D11_BUFFER_UAV_FLAG_COUNTER;
	//SCG->fragmentsBuffer->initDefaultUnorderedAccess(device, m_fragmentsBufferSize, fragmentSize, fragmentsBufferFlags);
	auto structsize = 4 + 4 + 4;
	auto count = backBufferSurfaceDesc->Width *  backBufferSurfaceDesc->Height * 32;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = count * structsize;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structsize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	
	hr = device->CreateBuffer(&desc, 0, SCG->fragmentsBuffer.ReleaseAndGetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
	srvdesc.Format = DXGI_FORMAT_UNKNOWN;

	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvdesc.Buffer.FirstElement = 0;
	srvdesc.Buffer.NumElements = count;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavdesc;
	uavdesc.Format = DXGI_FORMAT_UNKNOWN;

	uavdesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavdesc.Buffer.FirstElement = 0;
	uavdesc.Buffer.NumElements = count;
	uavdesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG::D3D11_BUFFER_UAV_FLAG_COUNTER;

	hr = device->CreateShaderResourceView( SCG->fragmentsBuffer.Get(), &srvdesc, SCG->fragmentsBufferSRV.ReleaseAndGetAddressOf());
	hr = device->CreateUnorderedAccessView(SCG->fragmentsBuffer.Get(), &uavdesc, SCG->fragmentsBufferUAV.ReleaseAndGetAddressOf());

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();

	delete SCG;
}
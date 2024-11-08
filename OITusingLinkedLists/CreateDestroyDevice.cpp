#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

HWND DXUTgetWindow();

GraphicResources * G;

SceneState scene_state;

std::unique_ptr<Keyboard> _keyboard;
std::unique_ptr<Mouse> _mouse;

CDXUTDialogResourceManager          g_DialogResourceManager;
CDXUTTextHelper*                    g_pTxtHelper = NULL;

Entity entityData[36];

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* device, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	srand((unsigned int)(0));

	HRESULT hr;
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();

	G = new GraphicResources();
	G->render_states = std::make_unique<CommonStates>(device);
	G->scene_constant_buffer = std::make_unique<ConstantBuffer<SceneState> >(device);

	_keyboard = std::make_unique<Keyboard>();
	_mouse = std::make_unique<Mouse>();
	HWND hwnd = DXUTgetWindow();
	_mouse->SetWindow(hwnd);

	g_DialogResourceManager.OnD3D11CreateDevice(device, context);
	g_pTxtHelper = new CDXUTTextHelper(device, context, &g_DialogResourceManager, 15);

	//effects
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"skybox.hlsl", L"SIMPLE_THROWPUT", L"vs_5_0" };
		shaderDef[L"GS"] = { L"skybox.hlsl", L"GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"skybox.hlsl", L"PS", L"ps_5_0" };

		G->skybox_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"opaque.hlsl", L"REF_MODEL", L"vs_5_0" };
		shaderDef[L"PS"] = { L"opaque.hlsl", L"REF_COLOR", L"ps_5_0" };

		G->opaque_ref_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"fragmentsListCreation.hlsl", L"REF_MODEL", L"vs_5_0" };
		shaderDef[L"PS"] = { L"fragmentsListCreation.hlsl", L"LIST_CREATION", L"ps_5_0" };

		G->fragments_list_creation_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"Quad.hlsl", L"VS", L"vs_5_0" };
		shaderDef[L"PS"] = { L"Quad.hlsl", L"PS", L"ps_5_0" };

		G->post_proccess_effect = createHlslEffect(device, shaderDef);
	}

	//textures
	{
		WCHAR wcPath[256];
		DXUTFindDXSDKMediaFileCch(wcPath, 256, L"Textures\\meadow.dds");
		hr = D3DX11CreateShaderResourceViewFromFile(device, wcPath, NULL, NULL, G->skybox_texture.ReleaseAndGetAddressOf(), NULL);
	}

	//meshes
	{
		G->teapot = GeometricPrimitive::CreateTeapot(context, 10, 8U, false);
	}

	//input layouts
	{
		G->teapot->CreateInputLayout(G->opaque_ref_effect.get(), G->pos_tex_normal_layout.ReleaseAndGetAddressOf());
	}

	//entity
	{
	const int ENTITIES_IN_ROW = 6;
	const float HALF_ENTITIES_IN_ROW = float(ENTITIES_IN_ROW) * 0.5f;
	const float AREA_HALFLENGTH = 45.0f;
	for (int i = 0; i < ENTITIES_IN_ROW; i++)
		{
			for (int j = 0; j < ENTITIES_IN_ROW; j++)
			{
				int index = i * ENTITIES_IN_ROW + j;
				float x = (float(i) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;
				float z = (float(j) - HALF_ENTITIES_IN_ROW) / HALF_ENTITIES_IN_ROW;

				entityData[index].transparent = !(i % 2 == 0 && j % 2 == 0);
				entityData[index].color = random();
				entityData[index].color.w = 0.3F + entityData[index].color.w * 0.4F;
				entityData[index].translation.x = x * AREA_HALFLENGTH;
				entityData[index].translation.y = 0.0f;
				entityData[index].translation.z = z * AREA_HALFLENGTH;
			}
		}
	}

	//light sources
		//directional light
		LightSource source;
		source.type = LightType::DirectLight;
		source.position = DirectX::XMFLOAT3(0, 15, 0);
			DirectX::XMVECTOR dir(XMLoadFloat3(&DirectX::XMFLOAT3(1, -1, 1)));
			dir = XMVector3Normalize(dir);
			DirectX::XMFLOAT3 _dir;
			XMStoreFloat3(&_dir, dir);
		source.orientation.set_from_axes(DirectX::XMFLOAT3(0, 0, 1), _dir);
		source.diffuseColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		source.specularColor = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);
		source.ambientColor = DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f);

		//directional light 2
		LightSource source2;
		source2.type = LightType::DirectLight;
		source2.position = DirectX::XMFLOAT3(15, 15, 0);
			DirectX::XMVECTOR dir2(XMLoadFloat3(&DirectX::XMFLOAT3(0, -1, 1)));
			dir2 = XMVector3Normalize(dir2);
			DirectX::XMFLOAT3 _dir2;
			XMStoreFloat3(&_dir2, dir2);
		source2.orientation.set_from_axes(DirectX::XMFLOAT3(0, 0, 1), _dir2);
		source2.diffuseColor = DirectX::XMFLOAT3(0.96f * 0.5f, 0.81f * 0.5f, 0.59f * 0.5f);
		source2.specularColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
		source2.ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	G->lightsBuffer = std::make_unique<framework::UniformBuffer>();
	if (!G->lightsBuffer->initDefaultStructured<LightRawData>(device, (size_t)16)) 
		return S_FALSE;

	G->lightsBuffer->setElement(0, lightSourceToRaw(source));
	G->lightsBuffer->setElement(1, lightSourceToRaw(source2));

	G->lightsBuffer->applyChanges(context);

	//alfa blending
	{
		D3D11_BLEND_DESC desc;

		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		for (int i = 0; i < 8; i++)
		{
			desc.RenderTarget[i].BlendEnable = true;
			desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		device->CreateBlendState(&desc, G->alpha_blending.ReleaseAndGetAddressOf());
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{

	delete g_pTxtHelper;

	g_DialogResourceManager.OnD3D11DestroyDevice();

	_mouse = 0;

	_keyboard = 0;

	delete G;
}

#include "GeometricPrimitive.h"
#include "Effects.h"
#include "DirectXHelpers.h"
#include "Model.h"
#include "CommonStates.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SimpleMath.h"

#include "DirectXMath.h"

#include "DXUT.h"

#include <wrl.h>

#include <map>
#include <algorithm>
#include <array>
#include <vector>
#include <memory>
#include <assert.h>
#include <malloc.h>
#include <Exception>

#include "ConstantBuffer.h"

#include "AppConstBuffer.h"

#include "resourceview.h"

#include "UniformBuffer.h"

#include "unorderedaccessbuffer.h"

#include "quaternion.h"

using namespace DirectX;

struct EffectShaderFileDef{
	WCHAR * name;
	WCHAR * entry_point;
	WCHAR * shader_ver;
};
class IPostProcess
{
public:
	virtual ~IPostProcess() { }

	virtual void __cdecl Process(_In_ ID3D11DeviceContext* deviceContext, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr) = 0;
};

inline ID3D11RenderTargetView** renderTargetViewToArray(ID3D11RenderTargetView* rtv1, ID3D11RenderTargetView* rtv2 = 0, ID3D11RenderTargetView* rtv3 = 0){
	static ID3D11RenderTargetView* rtvs[10];
	rtvs[0] = rtv1;
	rtvs[1] = rtv2;
	rtvs[2] = rtv3;
	return rtvs;
};
inline ID3D11UnorderedAccessView** UAVToArray(ID3D11UnorderedAccessView* rtv1, ID3D11UnorderedAccessView* rtv2 = 0, ID3D11UnorderedAccessView* rtv3 = 0){
	static ID3D11UnorderedAccessView* rtvs[10];
	rtvs[0] = rtv1;
	rtvs[1] = rtv2;
	rtvs[2] = rtv3;
	return rtvs;
};
inline ID3D11ShaderResourceView** shaderResourceViewToArray(ID3D11ShaderResourceView* rtv1, ID3D11ShaderResourceView* rtv2 = 0, ID3D11ShaderResourceView* rtv3 = 0){
	static ID3D11ShaderResourceView* srvs[10];
	srvs[0] = rtv1;
	srvs[1] = rtv2;
	srvs[2] = rtv3;
	return srvs;
};
inline ID3D11Buffer** constantBuffersToArray(DirectX::ConstantBuffer<SceneState> &cb){
	static ID3D11Buffer* cbs[10];
	cbs[0] = cb.GetBuffer();
	return cbs;
};
inline ID3D11SamplerState** samplerStateToArray(ID3D11SamplerState* ss1, ID3D11SamplerState* ss2 = 0){
	static ID3D11SamplerState* sss[10];
	sss[0] = ss1;
	sss[1] = ss2;
	return sss;
};

inline DirectX::XMFLOAT4 random(float minValue = 0.0, float maxValue = 1.0){
	float test = rand();
	float r1 = float(rand() % 1000) / float(999);
	float r2 = float(rand() % 1000) / float(999);
	float r3 = float(rand() % 1000) / float(999);
	float w = float(rand() % 1000) / float(999);
	float d = maxValue - minValue;
	return DirectX::XMFLOAT4(minValue + r1 * d, minValue + r2 * d, minValue + r3 * d, minValue + w * d);
}

namespace Camera{
	void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
}
std::unique_ptr<DirectX::IEffect> createHlslEffect(ID3D11Device* device, std::map<const WCHAR*, EffectShaderFileDef>& fileDef);

void skybox_set_world_matrix();
void skybox_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);
void DrawQuad(ID3D11DeviceContext* pd3dImmediateContext, _In_ IEffect* effect, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);

void teapot_set_world_matrix(DirectX::XMFLOAT3 translation);
void teapot_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);

void post_proccess(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);

class GraphicResources {
public:
	std::unique_ptr<CommonStates> render_states;

	Microsoft::WRL::ComPtr<ID3D11BlendState> alpha_blending;

	std::unique_ptr<DirectX::IEffect> skybox_effect, opaque_ref_effect, fragments_list_creation_effect, post_proccess_effect;

	std::unique_ptr<DirectX::ConstantBuffer<SceneState> > scene_constant_buffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skybox_texture;

	std::unique_ptr<GeometricPrimitive> teapot;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> pos_tex_normal_layout;

	std::unique_ptr<framework::UniformBuffer> lightsBuffer;
};

class Entity{
public:
	DirectX::XMFLOAT3 translation;
	boolean transparent;
	DirectX::XMFLOAT4 color;
};

class SwapChainGraphicResources {
public:
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > depthStencilSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView> > depthStencilV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D> > depthStencilT;

	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > colorLayerSRV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView> > colorLayerV;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D> > colorLayerT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> headBufferT;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> headBufferSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> headBufferUAV;

	Microsoft::WRL::ComPtr<ID3D11Buffer> fragmentsBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fragmentsBufferSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> fragmentsBufferUAV;
};

enum LightType
{
	OmniLight = 0,
	SpotLight,
	DirectLight
};

struct LightSource
{
	LightType type;
	DirectX::XMFLOAT3 diffuseColor;
	DirectX::XMFLOAT3 ambientColor;
	DirectX::XMFLOAT3 specularColor;
	float falloff;
	DirectX::XMFLOAT3 position;
	quaternion orientation;
	float angle;

	LightSource() : type(LightType::OmniLight), diffuseColor(1.0f, 1.0f, 1.0f), ambientColor(0.3f, 0.3f, 0.3f),
		specularColor(1.0f, 1.0f, 1.0f), falloff(1000.0f), position(0.0f, 0.0f, 0.0f), angle(60.0f) {}
};

#pragma pack (push, 1)
struct LightRawData
{
	DirectX::XMFLOAT3 position;
	unsigned int lightType;
	DirectX::XMFLOAT3 direction;
	float falloff;
	DirectX::XMFLOAT3 diffuseColor;
	float angle;
	DirectX::XMFLOAT3 ambientColor;
	unsigned int : 32;
	DirectX::XMFLOAT3 specularColor;
	unsigned int : 32;
};
#pragma pack (pop)

inline LightRawData lightSourceToRaw(LightSource & source){
	LightRawData result;
	DirectX::XMFLOAT3 dir = source.orientation.z_direction();
	result.position = source.position;
	result.direction = dir;
	result.lightType = (unsigned int)source.type;
	result.diffuseColor = source.diffuseColor;
	result.falloff = source.falloff;
	result.ambientColor = source.ambientColor;
	result.angle = n_deg2rad(source.angle);
	result.specularColor = source.specularColor;

	return result;
}
#include "main.h"

extern GraphicResources * G;

extern SceneState scene_state;

void DrawQuad(ID3D11DeviceContext* pd3dImmediateContext, _In_ IEffect* effect,
	_In_opt_ std::function<void __cdecl()> setCustomState){
	effect->Apply(pd3dImmediateContext);
	setCustomState();

	pd3dImmediateContext->IASetInputLayout(nullptr);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pd3dImmediateContext->Draw(4, 0);
}

void DrawPoint(ID3D11DeviceContext* pd3dImmediateContext, _In_ IEffect* effect,
	_In_opt_ std::function<void __cdecl()> setCustomState){
	effect->Apply(pd3dImmediateContext);
	setCustomState();

	pd3dImmediateContext->IASetInputLayout(nullptr);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dImmediateContext->Draw(1, 0);
}

//skybox///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void skybox_set_world_matrix(){
	DirectX::XMFLOAT4X4 view;
	DirectX::XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mInvView)));

	DirectX::XMMATRIX wvp;
	wvp = XMMatrixTranslation(view._41, view._42, view._43);
	DirectX::XMStoreFloat4x4(&scene_state.mWorld, DirectX::XMMatrixTranspose(wvp));
	wvp = wvp * XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mView));
	DirectX::XMStoreFloat4x4(&scene_state.mWorldView, DirectX::XMMatrixTranspose(wvp));
	wvp = wvp * XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mProjection));
	DirectX::XMStoreFloat4x4(&scene_state.mWorldViewProjection, DirectX::XMMatrixTranspose(wvp));
}

void skybox_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	DrawPoint(pd3dImmediateContext, effect, [=]{
		setCustomState();
	});
}

//teapot///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void teapot_set_world_matrix(DirectX::XMFLOAT3 translation){
	DirectX::XMMATRIX wvp;
	wvp = XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(translation.x, translation.y, translation.z);
	DirectX::XMStoreFloat4x4(&scene_state.mWorld, DirectX::XMMatrixTranspose(wvp));
	wvp = wvp * XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mView));
	DirectX::XMStoreFloat4x4(&scene_state.mWorldView, DirectX::XMMatrixTranspose(wvp));
	wvp = wvp * XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mProjection));
	DirectX::XMStoreFloat4x4(&scene_state.mWorldViewProjection, DirectX::XMMatrixTranspose(wvp));
}

void teapot_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	G->teapot->Draw(effect, inputLayout, false, false, [=]{
		setCustomState();
	});
}

//teapot///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void post_proccess(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	DrawQuad(pd3dImmediateContext, effect, [=]{
		setCustomState();
	});
}
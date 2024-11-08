#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

extern GraphicResources * G;

extern SwapChainGraphicResources * SCG;

extern SceneState scene_state;
extern Entity entityData[36];
extern CDXUTTextHelper*                    g_pTxtHelper;

ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr };

void RenderText()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(true && DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	g_pTxtHelper->End();
}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	Camera::OnFrameMove(fTime, fElapsedTime, pUserContext);
}

void OnDrawScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext)
{
	OnDrawScene(pd3dDevice, pd3dImmediateContext, DXUTGetD3D11RenderTargetView(), DXUTGetD3D11DepthStencilView());

	RenderText();
}

void OnDrawScene(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV)
{
	HRESULT hr;

	// clear only head buffer
	unsigned int clearValue[4] = { -1, -1, -1, -1 };
	context->ClearUnorderedAccessViewUint(SCG->headBufferUAV.Get(), clearValue);

	// clear render target
	float ClearColor[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
	context->ClearRenderTargetView(pRTV, ClearColor);
	context->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// set render target and head/fragments buffers
	unsigned int initialCounts[2] = { 0, -1 };
	context->OMSetRenderTargetsAndUnorderedAccessViews(
		1, 
		renderTargetViewToArray(pRTV), 
		pDSV,
		1,
		2,
		UAVToArray(SCG->fragmentsBufferUAV.Get(), SCG->headBufferUAV.Get()),
		initialCounts
	);

	// render skybox
	if (true){
		skybox_set_world_matrix();

		G->scene_constant_buffer->SetData(context, scene_state);

		skybox_draw(context, G->skybox_effect.get(), 0, [=]{
			context->GSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

			context->PSSetShaderResources(0, 1, shaderResourceViewToArray(G->skybox_texture.Get()));

			context->PSSetSamplers(0, 1, samplerStateToArray(G->render_states->AnisotropicClamp()));

			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullClockwise());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});
	}

	// render opaque objects
	if (true){
		for (int i = 0; i < 36; i++){
			if (entityData[i].transparent) continue;

			teapot_set_world_matrix(entityData[i].translation);

			G->scene_constant_buffer->SetData(context, scene_state);

			teapot_draw(context, G->opaque_ref_effect.get(), G->pos_tex_normal_layout.Get(), [=]{
				context->VSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

				context->PSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

				context->PSSetShaderResources(0, 2, shaderResourceViewToArray(G->skybox_texture.Get(), G->lightsBuffer->getView().asShaderView()));

				context->PSSetSamplers(0, 1, samplerStateToArray(G->render_states->AnisotropicClamp()));

				context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
				context->RSSetState(G->render_states->CullCounterClockwise());
				context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
			});

		}
	}

	// build lists of fragments for transparent objects
	if (true){
		for (int i = 0; i < 36; i++){
			if (!entityData[i].transparent) continue;

			teapot_set_world_matrix(entityData[i].translation);

			G->scene_constant_buffer->SetData(context, scene_state);

			teapot_draw(context, G->fragments_list_creation_effect.get(), G->pos_tex_normal_layout.Get(), [=]{
				context->VSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

				context->PSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

				context->PSSetShaderResources(0, 2, shaderResourceViewToArray(G->skybox_texture.Get(), G->lightsBuffer->getView().asShaderView()));

				context->PSSetSamplers(0, 1, samplerStateToArray(G->render_states->AnisotropicClamp()));

				context->OMSetBlendState(G->render_states->Additive(), Colors::Black, 0xFFFFFFFF);
				context->RSSetState(G->render_states->CullNone());
				context->OMSetDepthStencilState(G->render_states->DepthRead(), 0);
			});

		}
	}

	// render transparent objects
	if (true){
		post_proccess(context, G->post_proccess_effect.get(), 0, [=]{
			context->OMSetBlendState(G->alpha_blending.Get(), Colors::White, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});
	}
}

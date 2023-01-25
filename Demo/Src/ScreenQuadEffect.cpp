#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders\Compiled\DeferredScreenQuadPS.h"
#include "Shaders\Compiled\DeferredScreenQuadVS.h"
}

using namespace DirectX;

void ScreenQuadEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_DeferredScreenQuadVS, sizeof(g_DeferredScreenQuadVS), 
		g_DeferredScreenQuadPS, sizeof(g_DeferredScreenQuadPS));
}
void ScreenQuadEffect::SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv)
{
	context->PSSetShaderResources(0, bufferCount, srv);
}
void ScreenQuadEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
}
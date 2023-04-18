#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders\Compiled\DeferredScreenQuadVS.h"
#include "Shaders\Compiled\AmbientScreenQuadPS.h"
}

void AOScreenQuadEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_DeferredScreenQuadVS, sizeof(g_DeferredScreenQuadVS),
		g_AmbientScreenQuadPS, sizeof(g_AmbientScreenQuadPS));

	m_CbPerFrame.Create(device);
}

void AOScreenQuadEffect::SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv)
{
	context->PSSetShaderResources(0, bufferCount, srv);
}

void AOScreenQuadEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSGetSamplers(0, 1, &sampler);
}

void AOScreenQuadEffect::SetAOData(float s, float k, float R)
{
	m_CbPerFrameData.s = s;
	m_CbPerFrameData.k = k;
	m_CbPerFrameData.R = R;
}

void AOScreenQuadEffect::SetScreenResolution(float width, float height)
{
	m_CbPerFrameData.width = width;
	m_CbPerFrameData.height = height;
}

void AOScreenQuadEffect::SetCameraPosition(DirectX::XMFLOAT3 camPos)
{
	m_CbPerFrameData.eyePos = camPos;
}

void AOScreenQuadEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}

void AOScreenQuadEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->PSSetConstantBuffers(0, 1, m_CbPerFrame.GetAddressOf());
}

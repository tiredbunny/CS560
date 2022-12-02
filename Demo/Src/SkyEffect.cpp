#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders/Compiled/CompiledSkyPS.h"
#include "Shaders/Compiled/CompiledSkyVS.h"
}

void SkyEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_SkyVS, sizeof(g_SkyVS), g_SkyPS, sizeof(g_SkyPS));
	m_CbPerObject.Create(device);
	m_CbPerFrame.Create(device);
}

void SkyEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbConstantsData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void SkyEffect::SetTextureCube(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void SkyEffect::SetSamplerState(ID3D11DeviceContext* context, ID3D11SamplerState* state)
{
	context->PSSetSamplers(0, 1, &state);
}

void SkyEffect::SetSkyBoxEnabled(bool status)
{
	if (status)
		m_CbPerFrameData.padding = DirectX::XMFLOAT2(1.0f, 1.0f);
	else
		m_CbPerFrameData.padding = DirectX::XMFLOAT2(0.0f, 0.0f);

}

void SkyEffect::SetScreenResolution(DirectX::XMFLOAT2 screenRes)
{
	m_CbPerFrameData.ScreenResolution = screenRes;
}

void SkyEffect::SetGradientColors(DirectX::XMFLOAT4 ColorA, DirectX::XMFLOAT4 ColorB)
{
	m_CbPerFrameData.ColorA = ColorA;
	m_CbPerFrameData.ColorB = ColorB;
}

void SkyEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbConstantsData);
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}

void SkyEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_CbPerFrame.GetAddressOf());
}

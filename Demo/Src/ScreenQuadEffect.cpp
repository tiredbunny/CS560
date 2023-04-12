#include "pch.h"
#include "Shaders.h"
#include "DemoScene.h"
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

	m_CbPerFrame.Create(device);
}
void ScreenQuadEffect::SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv)
{
	context->PSSetShaderResources(0, bufferCount, srv);
}
void ScreenQuadEffect::SetScreenResolution(float width, float height)
{
	m_CbPerFrameData.width = width;
	m_CbPerFrameData.height = height;
}
void ScreenQuadEffect::SetIRMapAndEnvMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* IRMap, ID3D11ShaderResourceView* EnvMap)
{
	ID3D11ShaderResourceView* maps[] = { IRMap, EnvMap };

	context->PSSetShaderResources(BUFFER_COUNT, 2, maps);
}


void ScreenQuadEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}
void ScreenQuadEffect::SetHammersleyData(HammerseleyData data)
{
	m_CbPerFrameData.N = data.N;

	for (int i = 0; i < 192; ++i)
		m_CbPerFrameData.hammersley[i] = 0;

	for (int i = 0; i < data.values.size(); ++i)
	{
		m_CbPerFrameData.hammersley[i] = data.values[i];
	}
}
void ScreenQuadEffect::SetCameraPosition(DirectX::XMFLOAT3 camPos)
{
	m_CbPerFrameData.CameraPosition = camPos;
}
void ScreenQuadEffect::SetGlobalLight(DirectX::XMFLOAT3 lightDir, DirectX::XMFLOAT3 lightColor)
{
	m_CbPerFrameData.LightColor = lightColor;
	m_CbPerFrameData.LightDir = lightDir;
}

void ScreenQuadEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}
void ScreenQuadEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->PSSetConstantBuffers(0, 1, m_CbPerFrame.GetAddressOf());
}
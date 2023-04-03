#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders\Compiled\LocalLightVS.h"
#include "Shaders\Compiled\LocalLightPS.h"

}

using namespace DirectX;

void LocalLightEffect::SetPBRProperties(float metallic, float roughness, float ao)
{
	m_CbPBRData.metallic = metallic;
	m_CbPBRData.roughness = roughness;
	m_CbPBRData.ao = ao;
}

void LocalLightEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void LocalLightEffect::SetCameraPosition(DirectX::XMFLOAT3 position)
{
	m_CbPerFrameData.CameraPosition = position;
}

void LocalLightEffect::SetLightData(DirectX::XMFLOAT3 lightPos, DirectX::XMFLOAT3 lightColor, float radius)
{
	m_CbPerFrameData.LightColor = lightColor;
	m_CbPerFrameData.LightPosition = lightPos;
	m_CbPerFrameData.radius = radius;
}

void LocalLightEffect::SetVisualizeSphere(bool enabled)
{
	if (enabled)
		m_CbPerFrameData.visualizeSphere = 1.0f;
	else
		m_CbPerFrameData.visualizeSphere = 0.0f;
}

void LocalLightEffect::SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv)
{
	context->PSSetShaderResources(0, bufferCount, srv);
}

void LocalLightEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_LocalLightVS, sizeof(g_LocalLightVS), 
		g_LocalLightPS, sizeof(g_LocalLightPS));
	m_CbPerFrame.Create(device);
	m_CbPerObject.Create(device);
	m_CbPBR.Create(device);
}

void LocalLightEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
	m_CbPBR.SetData(context, m_CbPBRData);
}

void LocalLightEffect::ApplyPerFrameConstants(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}

void LocalLightEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);

	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());

	ID3D11Buffer* bufs[] = { m_CbPerFrame.Get(), m_CbPBR.Get() };
	context->PSSetConstantBuffers(0, 2, bufs);
}

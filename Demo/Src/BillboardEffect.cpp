#include "pch.h"
#include "Shaders.h"

namespace
{
	#include "Shaders\Compiled\BillboardVS.h"
	#include "Shaders\Compiled\BillboardGS.h"
	#include "Shaders\Compiled\BillboardPS.h"
}

using namespace DirectX;

void BillboardEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, 
		g_BillboardVS, sizeof(g_BillboardVS),
		g_BillboardPS, sizeof(g_BillboardPS),
		g_BillboardGS, sizeof(g_BillboardGS));

	m_CbPerFrame.Create(device);
	m_CbPerObject.Create(device);
}

void BillboardEffect::SetEyePos(DirectX::FXMVECTOR eyePos)
{
	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePos);
}

void BillboardEffect::ApplyPerFrameConstants(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}

void BillboardEffect::SetViewProj(DirectX::FXMMATRIX viewProj)
{
	XMStoreFloat4x4(&m_CbPerObjectData.ViewProj, viewProj);
}

void BillboardEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
}

void BillboardEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}

void BillboardEffect::SetTextureArray(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void BillboardEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);

	ID3D11Buffer* buffers[] = { m_CbPerFrame.Get(), m_CbPerObject.Get() };
	context->GSSetConstantBuffers(0, 2, buffers);
}
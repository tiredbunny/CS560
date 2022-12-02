#include "pch.h"
#include "Sky.h"
#include "Vertex.h"
#include <DDSTextureLoader.h>
#include "GeometricPrimitive.h"
#include <string>

using namespace DirectX;

namespace
{
#include "Shaders/Compiled/CompiledSkyVS.h"
}

void Sky::Create(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
{
	assert(device != nullptr);

	DX::ThrowIfFailed
	(
		CreateDDSTextureFromFile(device, cubemapFilename.c_str(), nullptr,
			m_TextureCubeSRV.ReleaseAndGetAddressOf())

	);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	DX::ThrowIfFailed
	(
		device->CreateInputLayout(SkyVertex::InputElements, SkyVertex::ElementCount,
			g_SkyVS, sizeof(g_SkyVS), inputLayout.ReleaseAndGetAddressOf())
	);

	m_SkyEffect.Create(device, inputLayout);

	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;
	GeometricPrimitive::CreateSphere(vertices, indices, 2.0f * skySphereRadius, 30, false);

	std::vector<SkyVertex> skyVertices;
	//convert to skyVertex 
	for (auto const& vertex : vertices)
	{
		skyVertices.push_back(SkyVertex{ vertex.position });
	}

	Helpers::CreateMeshBuffer(device, skyVertices, D3D11_BIND_VERTEX_BUFFER, m_VertexBuffer.ReleaseAndGetAddressOf());
	Helpers::CreateMeshBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, m_IndexBuffer.ReleaseAndGetAddressOf());

	m_IndexCount = static_cast<UINT>(indices.size());

	//create DSS
	CD3D11_DEFAULT d3dDefault;
	{
		CD3D11_DEPTH_STENCIL_DESC desc(d3dDefault);

		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		DX::ThrowIfFailed
		(
			device->CreateDepthStencilState(&desc, m_LessEqualDSS.ReleaseAndGetAddressOf())
		);
	}
}

void Sky::Draw(ID3D11DeviceContext* context, ID3D11SamplerState* sampler, ID3D11RasterizerState* NoCull,
	XMFLOAT3 eyePos,
	FXMMATRIX viewProj, UINT screenWidth,
	UINT screenHeight, bool useSkybox,
	DirectX::XMFLOAT4 ColorA, DirectX::XMFLOAT4 ColorB)
{
	UINT stride = sizeof(SkyVertex);
	UINT offset = 0;

	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, viewProj);

	m_SkyEffect.SetWorldViewProj(WVP);
	m_SkyEffect.SetTextureCube(context, m_TextureCubeSRV.Get());
	m_SkyEffect.SetSamplerState(context, sampler);
	m_SkyEffect.SetScreenResolution(XMFLOAT2((float)screenWidth, (float)screenHeight));
	m_SkyEffect.SetSkyBoxEnabled(useSkybox);

	m_SkyEffect.SetGradientColors(ColorA, ColorB);

	m_SkyEffect.Bind(context);
	m_SkyEffect.Apply(context);

	context->RSSetState(NoCull);
	context->OMSetDepthStencilState(m_LessEqualDSS.Get(), 0);

	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(m_IndexCount, 0, 0);

	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);

}

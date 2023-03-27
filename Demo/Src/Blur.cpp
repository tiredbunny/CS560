#include "pch.h"
#include "Blur.h"

namespace
{
#include "Shaders/Compiled/HorzBlurCS.h"
#include "Shaders/Compiled/VertBlurCS.h"
}

BlurFilter::BlurFilter(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format) :
	m_Width(width), m_Height(height)
{
	D3D11_TEXTURE2D_DESC blurredTexDesc = {};
	blurredTexDesc.Width = width;
	blurredTexDesc.Height = height;
	blurredTexDesc.MipLevels = 1;
	blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format = format;
	blurredTexDesc.SampleDesc.Count = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
	blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
	blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	blurredTexDesc.CPUAccessFlags = 0;
	blurredTexDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> blurredTex;
	DX::ThrowIfFailed(device->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	DX::ThrowIfFailed(device->CreateShaderResourceView(blurredTex.Get(), &srvDesc, &m_OutputSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	DX::ThrowIfFailed(device->CreateUnorderedAccessView(blurredTex.Get(), &uavDesc, &m_OutputUAV));

	DX::ThrowIfFailed(device->CreateComputeShader(g_HorzBlurCS, sizeof(g_HorzBlurCS), nullptr, &m_HorzBlurCS));
	DX::ThrowIfFailed(device->CreateComputeShader(g_VertBlurCS, sizeof(g_VertBlurCS), nullptr, &m_VertBlurCS));
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV)
{
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };

	//============================= Horizontal Pass =============================================//

	dc->CSSetShader(m_HorzBlurCS.Get(), nullptr, 0);
	
	dc->CSSetShaderResources(0, 1, &inputSRV);
	ID3D11UnorderedAccessView* uavs[1] = { m_OutputUAV.Get()};
	dc->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

	UINT numGroupsX = (UINT)ceilf(m_Width / 256.0f);
	dc->Dispatch(numGroupsX, m_Height, 1);

	dc->CSSetShaderResources(0, 1, nullSRV);
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	////============================== Vertical Pass ===============================================//

	dc->CSSetShader(m_VertBlurCS.Get(), nullptr, 0);

	ID3D11ShaderResourceView* srvs[] = { m_OutputSRV.Get() };
	dc->CSSetShaderResources(0, 1, srvs);
	uavs[0] = inputUAV;
	dc->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

	UINT numGroupsY = (UINT)ceilf(m_Height / 256.0f);
	dc->Dispatch(m_Width, numGroupsY, 1);

	dc->CSSetShaderResources(0, 1, nullSRV);
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	////============================================================================================

	dc->CSSetShader(nullptr, nullptr, 0);
}

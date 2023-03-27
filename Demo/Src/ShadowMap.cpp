#include "pch.h"
#include "ShadowMap.h"


ShadowMap::ShadowMap(ID3D11Device* device, UINT width, UINT height)
    : m_Width(width), m_Height(height), m_DepthMapDSV(0)
{
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;
    m_Viewport.Width = static_cast<float>(width);
    m_Viewport.Height = static_cast<float>(height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;


    //================================ create depth buffer and depth stencil view ==================================//
    
    // Use typeless format because the DSV is going to interpret
    // the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
    // the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = m_Width;
    texDesc.Height = m_Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; /*| D3D11_BIND_SHADER_RESOURCE;*/
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
    DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, 0, &depthMap));

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    DX::ThrowIfFailed(device->CreateDepthStencilView(depthMap.Get(), &dsvDesc, &m_DepthMapDSV));

 /*   D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    DX::ThrowIfFailed(device->CreateShaderResourceView(depthMap.Get(), &srvDesc, &m_SingleChannelDepthMapSRV));*/


    //=============================== create frame buffer and render target view ====================================//

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tempTexture;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    DX::ThrowIfFailed(
        device->CreateTexture2D(&texDesc, NULL, &tempTexture)
    );


    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc32 = {};
    renderTargetViewDesc32.Format = texDesc.Format;
    renderTargetViewDesc32.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc32.Texture2D.MipSlice = 0;

    
    //Create Render target view
    DX::ThrowIfFailed(
        device->CreateRenderTargetView(tempTexture.Get(), &renderTargetViewDesc32, &m_DepthMapRTV)
	);

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = texDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    //Create SRV
	DX::ThrowIfFailed(
		device->CreateShaderResourceView(tempTexture.Get(), &shaderResourceViewDesc, &m_DepthMapSRV)
	);
    

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = texDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    
    //Create UAV
    DX::ThrowIfFailed(
        device->CreateUnorderedAccessView(tempTexture.Get(), &uavDesc, &m_DepthMapUAV)
    );

    CD3D11_DEFAULT d3dDefault;
    {
        CD3D11_RASTERIZER_DESC desc(d3dDefault);

        desc.DepthBias = 100000;
        desc.DepthBiasClamp = 0.0f;;
        desc.SlopeScaledDepthBias = 1.0f;

        DX::ThrowIfFailed
        (
            device->CreateRasterizerState(&desc, m_DepthRSS.ReleaseAndGetAddressOf())
        );
    }
    {
        CD3D11_SAMPLER_DESC desc(d3dDefault);

        desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        desc.BorderColor[0] = 0.0f;
        desc.BorderColor[1] = 0.0f;
        desc.BorderColor[2] = 0.0f;
        desc.BorderColor[3] = 0.0f;

        desc.ComparisonFunc = D3D11_COMPARISON_LESS;

        DX::ThrowIfFailed(
            device->CreateSamplerState(&desc, m_SamplerShadow.ReleaseAndGetAddressOf())
        );
    }
}

ShadowMap::~ShadowMap()
{

}

ID3D11ShaderResourceView* ShadowMap::GetDepthMapSRV()
{
    return m_DepthMapSRV.Get();
}

ID3D11UnorderedAccessView* ShadowMap::GetDepthMapUAV()
{
    return m_DepthMapUAV.Get();
}

void ShadowMap::BindDSVAndRTV(ID3D11DeviceContext* dc)
{
    dc->RSSetViewports(1, &m_Viewport);

    // Set null render target because we are only going to draw to depth buffer.
    // Setting a null render target will disable color writes.
    ID3D11RenderTargetView* renderTargets[1] = { m_DepthMapRTV.Get() };
    dc->OMSetRenderTargets(1, renderTargets, m_DepthMapDSV.Get());

    const float color[4] = { 1.0f,1.0f,1.0f,1.0f };
    dc->ClearRenderTargetView(m_DepthMapRTV.Get(), color);

    dc->ClearDepthStencilView(m_DepthMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


}

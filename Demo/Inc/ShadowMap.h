#pragma once

class ShadowMap
{
public:
	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap& operator=(const ShadowMap& rhs) = delete;

	ShadowMap(ID3D11Device* device, UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView* GetDepthMapShaderResourceView();

	ID3D11SamplerState* GetShadowSampler() const { return m_SamplerShadow.Get(); }
	ID3D11RasterizerState* GetDepthRSS() const { return m_DepthRSS.Get(); }

	void BindDSVAndSetNullRenderTarget(ID3D11DeviceContext* context);
private:
	UINT m_Width;
	UINT m_Height;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthMapDSV;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_DepthRSS;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerShadow;
	D3D11_VIEWPORT m_Viewport;
};
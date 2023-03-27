#pragma once

class BlurFilter
{
public:
	BlurFilter(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);
	~BlurFilter() = default;

	void BlurInPlace(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* inputSRV, ID3D11UnorderedAccessView* inputUAV);
public:
	UINT m_Width, m_Height;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_OutputSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_OutputUAV;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_HorzBlurCS;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_VertBlurCS;
};
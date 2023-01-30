/// ---------------------------------------------------------------------------
// File Name        :    Sky.h
// Author           :    Abhijit Zala
// Creation Date    :    9/6/22
// Purpose          :    Sky class
// History          :
// ---------------------------------------------------------------------------
#pragma once

#include "Shaders.h"
#include <string>

class Sky
{
public:
	Sky(const Sky& rhs) = delete;
	Sky& operator=(const Sky& rhs) = delete;

	~Sky() = default;
	Sky() = default;


	void Create(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius);

	void Draw(ID3D11DeviceContext* context, ID3D11SamplerState* sampler, ID3D11RasterizerState* NoCull,
		DirectX::XMFLOAT3 eyePos,
		DirectX::FXMMATRIX viewProj, UINT screenWidth, UINT screenHeight,
		bool useSkybox, DirectX::XMFLOAT4 ColorA, DirectX::XMFLOAT4 ColorB);
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_TextureCubeSRV;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_LessEqualDSS;

	SkyEffect m_SkyEffect;

	UINT m_IndexCount;
};
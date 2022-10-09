#pragma once

#include <wrl\client.h>
#include <d3d11.h>

template<typename T>
class ConstantBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;

public:
	ConstantBuffer() = default;
	ConstantBuffer(ConstantBuffer const&) = delete;
	ConstantBuffer& operator=(ConstantBuffer const&) = delete;

	explicit ConstantBuffer(ID3D11Device* device)
	{
		Create(device);
	}
	
	void Create(ID3D11Device* device) 
	{
		assert(device != nullptr);

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.ByteWidth = sizeof(T);

		DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, m_ConstantBuffer.ReleaseAndGetAddressOf()));
	}

	void SetData(ID3D11DeviceContext* context, T const& data) 
	{
		assert(context != nullptr);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		DX::ThrowIfFailed(context->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));

		*static_cast<T*>(mappedRes.pData) = data;
		
		context->Unmap(m_ConstantBuffer.Get(), 0);
	}

	ID3D11Buffer* Get() const
	{
		return m_ConstantBuffer.Get();
	}

	ID3D11Buffer* const* GetAddressOf() const
	{
		return m_ConstantBuffer.GetAddressOf();
	}
};
#pragma once

//Helper struct quickly put together to make code a bit cleaner while drawing
struct Drawable
{
	~Drawable() = default;
	Drawable(Drawable const&) = delete;
	Drawable& operator=(Drawable const&) = delete;

	Drawable() :
		IndexCount(0), VertexCount(0),
		IndexBufferFormat(DXGI_FORMAT_R16_UINT)
	{
		XMStoreFloat4x4(&WorldTransform, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&TextureTransform, DirectX::XMMatrixIdentity());
	}

	template<typename VertexType, typename IndexType>
	void Create(ID3D11Device * device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
	{
		assert(device != nullptr);
		IndexCount = static_cast<uint32_t>(indices.size());
		VertexCount = static_cast<uint32_t>(vertices.size());

		if (sizeof(IndexType) == 4)
			IndexBufferFormat = DXGI_FORMAT_R32_UINT;
		else
			static_assert(sizeof(IndexType) == 2, "Unknown index buffer format");

		Helpers::CreateMeshBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, VertexBuffer.ReleaseAndGetAddressOf());
		Helpers::CreateMeshBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, IndexBuffer.ReleaseAndGetAddressOf());
	}

	DirectX::XMMATRIX GetWorld() const { return XMLoadFloat4x4(&WorldTransform); }

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	uint32_t IndexCount;
	uint32_t VertexCount;
	DXGI_FORMAT IndexBufferFormat;
	DirectX::XMFLOAT4X4 WorldTransform;
	DirectX::XMFLOAT4X4 TextureTransform;
	Material Material;
};


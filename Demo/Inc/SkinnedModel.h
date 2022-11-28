#pragma once

#include "SkinnedData.h"
#include "MeshGeometry.h"
#include "Vertex.h"

class SkinnedModel
{
public:
	SkinnedModel(ID3D11Device* device, const std::string& modelFilename, const std::wstring& texturePath);
	~SkinnedModel();

	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<SkinnedVertex> Vertices;
	std::vector<USHORT> Indices;
	std::vector<MeshGeometry::Subset> Subsets;

	MeshGeometry ModelMesh;
	SkinnedData SkinnedData;
};

struct SkinnedModelInstance
{
	SkinnedModel* Model;
	float TimePos;
	std::string ClipName;
	DirectX::XMFLOAT4X4 World;
	std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
	
	std::vector<DirectX::XMFLOAT4> BonePositions;

	void Update(float dt, bool CCD, DirectX::XMFLOAT3 target);
};

#include "pch.h"
#include "SkinnedModel.h"
#include "LoadM3d.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

using namespace DirectX;

SkinnedModel::SkinnedModel(ID3D11Device* device, const std::string& modelFilename, const std::wstring& texturePath)
{
	std::vector<M3dMaterial> mats;
	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(modelFilename, Vertices, Indices, Subsets, mats, SkinnedData);

	ModelMesh.SetVertices(device, &Vertices[0], Vertices.size());
	ModelMesh.SetIndices(device, &Indices[0], Indices.size());
	ModelMesh.SetSubsetTable(Subsets);

	SubsetCount = mats.size();

	for (UINT i = 0; i < SubsetCount; ++i)
	{
		Mat.push_back(mats[i].Mat);

		ID3D11ShaderResourceView* diffuseMapSRV = nullptr;
		ID3D11ShaderResourceView* normalMapSRV = nullptr;

		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device,
				(texturePath + mats[i].DiffuseMapName).c_str(),
				0,
				&diffuseMapSRV));
			
		DiffuseMapSRV.push_back(diffuseMapSRV);

		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device,
				(texturePath + mats[i].NormalMapName).c_str(),
				0,
				&normalMapSRV));

		NormalMapSRV.push_back(normalMapSRV);
	}
}

SkinnedModel::~SkinnedModel()
{
}

void SkinnedModelInstance::Update(float dt)
{
	TimePos += dt;
	Model->SkinnedData.GetFinalTransforms(ClipName, TimePos, FinalTransforms, BonePositions);

	// Loop animation
	if (TimePos > Model->SkinnedData.GetClipEndTime(ClipName))
		TimePos = 0.0f;
}
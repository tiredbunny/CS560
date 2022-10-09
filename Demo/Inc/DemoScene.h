#pragma once

#include "DemoBase.h"
#include "ConstantBuffer.h"
#include "Shaders.h"
#include "SkinnedModel.h"
struct Drawable;

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;

	std::unique_ptr<Drawable> m_DrawableBox;
	std::unique_ptr<Drawable> m_DrawableSphere;
	std::unique_ptr<Drawable> m_DrawableTorus;
	std::unique_ptr<Drawable> m_DrawableTeapot;
	std::unique_ptr<Drawable> m_DrawableGrid;


	std::unique_ptr<SkinnedModel> m_SkinnedModel;
	SkinnedModelInstance m_SkinnedModelInstance;

	BasicEffect m_BasicEffect;
	BasicSkinnedEffect m_SkinnedEffect;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerAnisotropic;

	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSTransparent;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSNoColorWrite;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSAlphaToCoverage;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSCullNone;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSWireframe;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSFrontCounterCW;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSNoDepthWrite;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSMarkPixels;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSDrawMarkedOnly;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSNoDoubleBlend;

	DirectX::XMFLOAT4X4 m_CameraView;
	DirectX::XMFLOAT4X4 m_CameraProjection;

	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;


public:
	explicit DemoScene(const HWND& hwnd);
	DemoScene(const DemoScene&) = delete;
	~DemoScene() = default;

	bool Initialize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;
private:
	void Clear();
	void Present();
	bool CreateDeviceDependentResources();
	void CreateBuffers();
	void PrepareForRendering();
	void ResetStates();
	void FillBasicEffect(Drawable* drawable);
	
};
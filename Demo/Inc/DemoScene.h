#pragma once

#include "DemoBase.h"
#include "ConstantBuffer.h"
#include "Shaders.h"
#include "CommonStates.h"
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "Camera.h"
#include "Sky.h"
#include "ShadowMap.h"
#include "Blur.h"
#include <DirectXCollision.h>

struct Drawable;

auto constexpr BUFFER_COUNT = 4;

struct LocalLight
{
	DirectX::XMFLOAT3 LightPos;
	DirectX::XMFLOAT3 LightColor;
	float range;
};

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;

	std::unique_ptr<Drawable> m_DrawableGrid;
	std::unique_ptr<Drawable> m_DrawableSphere;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSFrontCounterCW;

	DirectionalLight m_DirLight;

	//Skybox
	Sky m_Sky;
	
	//deferred shading stuff
	ID3D11RenderTargetView* renderTargetViewArray[BUFFER_COUNT];
	ID3D11ShaderResourceView* shaderResourceViewArray[BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11Buffer> screenQuadVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> screenQuadIB;

	RenderGBuffersEffect m_BasicEffect;
	ScreenQuadEffect m_ScreenQuadEffect;
	LocalLightEffect m_LocalLightEffect;

	std::vector<LocalLight> m_LocalLights;

	//Sphere mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> sphereMeshVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sphereMeshIB;
	UINT sphereMeshIndexCount;

	//Shadows stuff
	std::unique_ptr<ShadowMap> m_ShadowMap;
	std::unique_ptr<BlurFilter> m_BlurFilter;
	DirectX::BoundingSphere m_SceneBounds;

	static const int m_ShadowMapSize = 2048;
	DirectX::XMFLOAT4X4 m_LightView;
	DirectX::XMFLOAT4X4 m_LightProj;
	DirectX::XMFLOAT4X4 m_ShadowTransform;

	std::unique_ptr<ShadowMapEffect> m_ShadowEffect;

	//Basic effect stuff for debug drawing
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_PrimitiveBatch;
	std::unique_ptr<DirectX::BasicEffect> m_DebugBasicEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_DebugBasicEffectInputLayout;

	std::unique_ptr<DirectX::CommonStates> m_CommonStates;

	Camera m_Camera;
	POINT m_LastMousePos;
public:
	explicit DemoScene(const HWND& hwnd);
	DemoScene(const DemoScene&) = delete;
	~DemoScene();

	bool Initialize() override;
	void UpdateScene(DX::StepTimer timer) override;
	void DrawScene() override;

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
private:
	void Clear();
	void Present();
	bool CreateDeviceDependentResources();
	void CreateBuffers();
	void CreateDeferredBuffers();
	void PrepareForRendering();
	void ResetStates();

	void ComputeShadowTransform();
	void RenderToShadowMap();
};
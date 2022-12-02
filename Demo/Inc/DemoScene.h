#pragma once

#include "DemoBase.h"
#include "ConstantBuffer.h"
#include "Shaders.h"
#include "SkinnedModel.h"
#include "CommonStates.h"
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include "Camera.h"
#include "Path.h"
#include "Sky.h"

struct Drawable;

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;

	std::unique_ptr<Drawable> m_DrawableGrid;
	std::unique_ptr<Drawable> m_DrawableSphere;


	std::unique_ptr<SkinnedModel> m_SkinnedModel;
	SkinnedModelInstance m_SkinnedModelInstance;

	BasicLightsEffect m_BasicEffect;
	BasicSkinnedEffect m_SkinnedEffect;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSFrontCounterCW;


	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;

	//Skybox
	Sky m_Sky;

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
	~DemoScene() = default;

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
	void PrepareForRendering();
	void ResetStates();
	void FillBasicEffect(Drawable* drawable);
	
};
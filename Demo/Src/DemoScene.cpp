#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Drawable.h"
#include "Vertex.h"
#include "Camera.h"
#include "MathHelper.h"
#include <VertexTypes.h>
#include <DirectXHelpers.h>
#include <DebugDraw.h>
#include "Helpers.h"

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;


extern DX::StepTimer g_Timer;

//These headers may not be present, so you may need to build the project once
namespace
{
	#include "Shaders\Compiled\BasicVS.h"
	#include "Shaders\Compiled\DeferredScreenQuadVS.h"
	#include "Shaders\Compiled\LocalLightVS.h"
}

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_DrawableGrid = std::make_unique<Drawable>();
	m_DrawableTetrahedron = std::make_unique<Drawable>();

	m_SceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_SceneBounds.Radius = 52.0f;
	
	//Setup DirectionalLight
	m_DirLight.SetDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));

	m_Camera.SetPosition(-2.0f, 5.0f, -20.0f);

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		m_DeferredRTV[i] = nullptr;
		m_DeferredSRV[i] = nullptr;
	}

	PBRMaterial mat;

	for (int i = 0; i < 10; ++i)
	{
		mat.Metallic = 0;

		for (int j = 0; j < 10; ++j)
		{
			mat.Roughness = 1;

			LocalLight light = {};

			light.LightPos = XMFLOAT3((i * 10) - 50.0f, 4.0f, (j * 10) - 40.0f);
			light.LightColor = XMFLOAT3(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());
			light.Range = MathHelper::RandF(10.0f, 40.0f);
			
			mat.SolidColor = XMFLOAT4(MathHelper::RandF(0.0f, 10.0f), MathHelper::RandF(0.0f, 10.0f), MathHelper::RandF(0.0f, 10.0f), 1.0f);

			m_LocalLights.push_back(light);
			m_Materials.push_back(mat);
		}

	}
	
	HammersleyBlockSetup();
}

DemoScene::~DemoScene()
{
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		m_DeferredRTV[i]->Release();
		m_DeferredSRV[i]->Release();
	}
}

bool DemoScene::CreateDeviceDependentResources()
{

	m_Sky.Create(m_Device.Get(), L"Textures\\grasscube1024.dds", 3000.0f);

	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\skybox1IR.dds", 0, m_IRCubeSRV.ReleaseAndGetAddressOf())
	);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosNormalTex;
	DX::ThrowIfFailed
	(
		m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements,
		GeometricPrimitive::VertexType::InputElementCount,
		g_BasicVS, sizeof(g_BasicVS),
		layoutPosNormalTex.ReleaseAndGetAddressOf())
	);
	m_BasicEffect.Create(m_Device.Get(), layoutPosNormalTex);


	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosTex;
	DX::ThrowIfFailed
	(
		m_Device->CreateInputLayout(ScreenQuadVertex::InputElements,
			ScreenQuadVertex::ElementCount,
			g_DeferredScreenQuadVS, sizeof(g_DeferredScreenQuadVS),
			layoutPosTex.ReleaseAndGetAddressOf())
	);
	m_ScreenQuadEffect.Create(m_Device.Get(), layoutPosTex);


	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPos;
	DX::ThrowIfFailed
	(
		m_Device->CreateInputLayout(VertexPosition::InputElements,
			VertexPosition::InputElementCount,
			g_LocalLightVS, sizeof(g_LocalLightVS),
			layoutPos.ReleaseAndGetAddressOf())
	);
	m_LocalLightEffect.Create(m_Device.Get(), layoutPos);


	m_ShadowMap = std::make_unique<ShadowMap>(m_Device.Get(), m_ShadowMapSize, m_ShadowMapSize);
	m_ShadowEffect = std::make_unique<ShadowMapEffect>(m_Device.Get(), layoutPosNormalTex);

	m_PrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_ImmediateContext.Get());
	m_DebugBasicEffect = std::make_unique<BasicEffect>(m_Device.Get());
	m_DebugBasicEffect->SetVertexColorEnabled(true);

	DX::ThrowIfFailed
	(
		CreateInputLayoutFromEffect<VertexPositionColor>(m_Device.Get(), m_DebugBasicEffect.get(), &m_DebugBasicEffectInputLayout)
	);

	m_CommonStates = std::make_unique<CommonStates>(m_Device.Get());

	m_BlurFilter = std::make_unique<BlurFilter>(m_Device.Get(), m_ShadowMapSize, m_ShadowMapSize, DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_BloomBlur = std::make_unique<BlurFilter>(m_Device.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
	
#pragma region Load Textures
	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\floor.dds", 0, m_DrawableGrid->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableTetrahedron->TextureSRV.ReleaseAndGetAddressOf()))
		return false;
#pragma endregion


	CreateGeometryBuffers();

	CreateDeferredBuffers();


	return true;
}

void DemoScene::CreateDeferredBuffers()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	
	textureDesc.Width = m_ClientWidth;
	textureDesc.Height = m_ClientHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> texArray(BUFFER_COUNT);

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		if (i == BUFFER_COUNT - 1)
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		DX::ThrowIfFailed(
			m_Device->CreateTexture2D(&textureDesc, NULL, &texArray[i])
			);
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;


	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		//Create Render target view
		DX::ThrowIfFailed(
			m_Device->CreateRenderTargetView(texArray[i].Get(), &renderTargetViewDesc, &m_DeferredRTV[i])
		);
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		DX::ThrowIfFailed(
			m_Device->CreateShaderResourceView(texArray[i].Get(), &shaderResourceViewDesc, &m_DeferredSRV[i])
		);
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	DX::ThrowIfFailed(
		m_Device->CreateUnorderedAccessView(texArray[BUFFER_COUNT - 1].Get(), &uavDesc, &m_BrightColorsUAV)
		);
}

void DemoScene::CreateGeometryBuffers()
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;
	
	Helpers::CreateGrid(vertices, indices, 100, 100);
	m_DrawableGrid->Create(m_Device.Get(), vertices, indices);


	std::vector<VertexPosition> sphereVertices;
	for (auto const& vertex : vertices)
	{
		sphereVertices.push_back(vertex.position);
	}
	m_SphereMeshIndexCount = indices.size();
	Helpers::CreateMeshBuffer(m_Device.Get(), sphereVertices, D3D11_BIND_VERTEX_BUFFER, &m_SphereMeshVB);
	Helpers::CreateMeshBuffer(m_Device.Get(), indices, D3D11_BIND_INDEX_BUFFER, &m_SphereMeshIB);


	GeometricPrimitive::CreateTeapot(vertices, indices, 6.0f, 8, false);
	m_DrawableTetrahedron->Create(m_Device.Get(), vertices, indices);

	ScreenQuadVertex screenQuadVertices[] =
	{
		XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f),
		XMFLOAT4(-1.0f, +1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f),
		XMFLOAT4(+1.0f, +1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f),
		XMFLOAT4(+1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)
	};

	short screenQuadIndices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	Helpers::CreateMeshBuffer(m_Device.Get(), screenQuadVertices, sizeof(screenQuadVertices) / sizeof(screenQuadVertices[0]), D3D11_BIND_VERTEX_BUFFER, m_ScreenQuadVB.ReleaseAndGetAddressOf());
	Helpers::CreateMeshBuffer(m_Device.Get(), screenQuadIndices, sizeof(screenQuadIndices) / sizeof(screenQuadIndices[0]), D3D11_BIND_INDEX_BUFFER, m_ScreenQuadIB.ReleaseAndGetAddressOf());
}

bool DemoScene::Initialize()
{
	if (!Super::Initialize())
		return false;
	
	if (!CreateDeviceDependentResources())
		return false;

	Super::ImGui_Init();

	return true;
}

void DemoScene::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(m_MainWindow);
}

void DemoScene::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DemoScene::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

		m_Camera.Pitch(dy);
		m_Camera.RotateY(dx);
	}

	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void DemoScene::UpdateScene(DX::StepTimer timer)
{
	float dt = timer.GetElapsedSeconds();

	Super::ImGui_NewFrame();

	m_Camera.SetLens(XMConvertToRadians(60.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 1000.0f);
	

	static float cameraSpeed = 40.0f;

	if (GetAsyncKeyState('W') & 0x8000)
		m_Camera.Walk(cameraSpeed * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Camera.Walk(-cameraSpeed * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Camera.Strafe(-cameraSpeed * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Camera.Strafe(cameraSpeed * dt);

	m_Camera.UpdateViewMatrix();

	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(20.0f, 20.0f, 0.0f));
}


void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::Black;
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<float*>(&backBufferColor));
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DemoScene::PrepareForRendering()
{
	m_ImmediateContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DemoScene::RenderToShadowMap()
{
	ComputeShadowTransform();

	XMMATRIX view = XMLoadFloat4x4(&m_LightView);
	XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);
	XMMATRIX viewProj = view * proj;

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	m_ShadowEffect->Bind(m_ImmediateContext.Get());

	static Drawable* drawables[] = { m_DrawableGrid.get() };
	for (auto const& it : drawables)
	{
		m_ShadowEffect->SetWorldViewProj(it->GetWorld() * viewProj);
		m_ShadowEffect->Apply(m_ImmediateContext.Get());

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	auto it = m_DrawableTetrahedron.get();
	m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);

	for (auto const& light : m_LocalLights)
	{
		XMMATRIX world = XMMatrixTranslation(light.LightPos.x, light.LightPos.y - 3.0f, light.LightPos.z);
		
		m_ShadowEffect->SetWorldViewProj(world * viewProj);
		m_ShadowEffect->Apply(m_ImmediateContext.Get());

		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//=================================== blur the rendered shadow map ======================================//

	ID3D11RenderTargetView* nullRTV[1] = { nullptr };
	m_ImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);

	static bool blurShadowMap = true;
	ImGui::Checkbox("Blur Shadow Map", &blurShadowMap);

	if (blurShadowMap)
		m_BlurFilter->BlurInPlace(m_ImmediateContext.Get(), m_ShadowMap->GetDepthMapSRV(), m_ShadowMap->GetDepthMapUAV());
}



void DemoScene::DrawScene()
{
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX viewProj = m_Camera.GetView() * m_Camera.GetProj();
	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };

	//====================================== Render to shadow map ===========================================//

	m_ShadowMap->BindDSVAndRTV(m_ImmediateContext.Get());
	m_ImmediateContext->RSSetState(m_ShadowMap->GetDepthRS());

	RenderToShadowMap();

	m_ImmediateContext->RSSetState(nullptr);
	m_ImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);

	//====================================== Render scene in G-buffers ======================================//
	
	//----------- imgui junk variables -------------------//
	static float range = 10.0f;
	static float sphereRadius = 10.0f;
	static float metallic = 0.0f;
	static float roughness = 0.5f;
	static float ao = 1.0f;
	static float gamma = 2.2f;
	static int bloomBlurSize = 1;
	static float bloomBrightness = 0.1f;
	//----------------------------------------------------//
	
	//
	// Restore the back and depth buffer to the OM stage.
	//
	m_ImmediateContext->RSSetViewports(1, &m_ScreenViewport);
	m_ImmediateContext->OMSetRenderTargets(BUFFER_COUNT, m_DeferredRTV, m_DepthStencilView.Get());

	for (int i = 0; i < BUFFER_COUNT; ++i)
		m_ImmediateContext->ClearRenderTargetView(m_DeferredRTV[i], DirectX::Colors::Black);
	
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_BasicEffect.Bind(m_ImmediateContext.Get());
	m_BasicEffect.SetSampler(m_ImmediateContext.Get(), m_CommonStates->AnisotropicWrap());
	m_BasicEffect.SetShadowTransform(XMLoadFloat4x4(&m_ShadowTransform));
	m_BasicEffect.SetShadowMap(m_ImmediateContext.Get(), m_ShadowMap->GetDepthMapSRV());
	m_BasicEffect.SetShadowSampler(m_ImmediateContext.Get(), m_ShadowMap->GetShadowSampler());
	m_BasicEffect.EnableMomentShadowMap(true);
	m_BasicEffect.SetSolidColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_BasicEffect.SetPBRProperties(metallic, roughness, ao, gamma);
	m_BasicEffect.SetBloomExtractBrightness(bloomBrightness);
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

	static Drawable* drawables[] = { m_DrawableGrid.get()};
	for (auto const& it : drawables)
	{
		m_BasicEffect.SetWorld(it->GetWorld());
		m_BasicEffect.SetWorldViewProj(it->GetWorld() * viewProj);
		m_BasicEffect.SetTextureTransform(XMLoadFloat4x4(&it->TextureTransform));
		m_BasicEffect.SetTexture(m_ImmediateContext.Get(), it->TextureSRV.Get());

		m_BasicEffect.Apply(m_ImmediateContext.Get());

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	auto it = m_DrawableTetrahedron.get();
	m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);

	//Draw spheres at each light position
	m_BasicEffect.SetTexture(m_ImmediateContext.Get(), it->TextureSRV.Get());
	for (int i = 0; i < m_LocalLights.size(); ++i)
	{
		XMMATRIX world = XMMatrixTranslation(m_LocalLights[i].LightPos.x, m_LocalLights[i].LightPos.y - 3.0f, m_LocalLights[i].LightPos.z);
		auto const& mat = m_Materials[i];
		
		m_BasicEffect.SetPBRProperties(mat.Metallic, mat.Roughness, mat.Ao, gamma);
		m_BasicEffect.SetWorld(world);
		m_BasicEffect.SetWorldViewProj(world * viewProj);	
		m_BasicEffect.SetSolidColor(m_Materials[i].SolidColor);
		m_BasicEffect.Apply(m_ImmediateContext.Get());
		m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	m_Sky.Draw(m_ImmediateContext.Get(), m_CommonStates.get(), m_Camera.GetPosition3f(), viewProj);

	//============================================ lighting pass ===================================================//
	
	Clear();
	PrepareForRendering();

	for (int i = 0; i < bloomBlurSize; ++i)
		m_BloomBlur->BlurInPlace(m_ImmediateContext.Get(), m_DeferredSRV[BUFFER_COUNT - 1], m_BrightColorsUAV.Get());

	//------------------ imgui junk ------------------------------------------------//
	ImGui::DragInt("Bloom Blur", &bloomBlurSize, 1.0f, 0.0f, 30);
	ImGui::DragFloat("Bloom Brightness", &bloomBrightness, 0.01f, 0.0f, 1.0f);
	//for (int i = 0; i < BUFFER_COUNT; ++i)
		ImGui::Image(m_DeferredSRV[BUFFER_COUNT - 1], ImVec2(400, 200));
	

	ImGui::DragFloat3("Global light direction", reinterpret_cast<float*>(&m_DirLight.Direction), 0.05f, -1.0f, 1.0f);
	ImGui::DragFloat("metallic", &metallic, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("roughness", &roughness, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("ao", &ao, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("gamma", &gamma, 0.01f, 1.0f, 2.2f);
	//ImGui::DragFloat("Range", &range, 0.5f, 1.0f, 1000.0f);
	//ImGui::DragFloat("sphereRadius", &sphereRadius, 0.5f, 1.0f, 1000.0f);
	ImGui::DragFloat3("Center##1", reinterpret_cast<float*>(&m_SceneBounds.Center), 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat("Radius", &m_SceneBounds.Radius, 1.0f, 1.0f, 9000.0f);
	ImGui::Image(m_ShadowMap->GetDepthMapSRV(), ImVec2(400, 200));

	//-----------------------------------------------------------------------------//

	float blend[4] = {1,1,1,1};
	m_ImmediateContext->OMSetBlendState(m_CommonStates->Additive(), blend, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(m_CommonStates->DepthNone(), 0);


	XMVECTOR normalized = XMVector3Normalize(XMLoadFloat3(&m_DirLight.Direction));
	XMFLOAT3 normalizedf;
	XMStoreFloat3(&normalizedf, normalized);

	//--------------------- Draw full-screen quad -----------------------------------//
	m_ScreenQuadEffect.Bind(m_ImmediateContext.Get());
	m_ScreenQuadEffect.SetGBuffers(m_ImmediateContext.Get(), BUFFER_COUNT, m_DeferredSRV);
	m_ScreenQuadEffect.SetIRMapAndEnvMap(m_ImmediateContext.Get(), m_IRCubeSRV.Get(), m_Sky.GetCubeMap());
	m_ScreenQuadEffect.SetSampler(m_ImmediateContext.Get(), m_CommonStates->LinearWrap());
	m_ScreenQuadEffect.SetGlobalLight(normalizedf, XMFLOAT3(1.0f, 1.0f, 1.0f));
	m_ScreenQuadEffect.SetCameraPosition(m_Camera.GetPosition3f());
	m_ScreenQuadEffect.SetHammersleyData(m_HammersleyData);
	m_ScreenQuadEffect.SetScreenResolution(m_ClientWidth, m_ClientHeight);
	m_ScreenQuadEffect.Apply(m_ImmediateContext.Get());

	stride = sizeof(ScreenQuadVertex);
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_ScreenQuadVB.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_ScreenQuadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_ImmediateContext->DrawIndexed(6, 0, 0);

	//----------------------- Draw local lights --------------------------------------//
	stride = sizeof(VertexPosition);
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_SphereMeshVB.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_SphereMeshIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	
	m_LocalLightEffect.Bind(m_ImmediateContext.Get());
	m_LocalLightEffect.SetCameraPosition(m_Camera.GetPosition3f());
	m_LocalLightEffect.SetGBuffers(m_ImmediateContext.Get(), BUFFER_COUNT, m_DeferredSRV);

	m_ImmediateContext->RSSetState(m_CommonStates->CullNone());
	for (auto const& light : m_LocalLights)
	{
		XMMATRIX world = XMMatrixScaling(sphereRadius, sphereRadius, sphereRadius) * XMMatrixTranslation(light.LightPos.x, light.LightPos.y, light.LightPos.z);

		m_LocalLightEffect.SetWorldViewProj(world * viewProj);
		m_LocalLightEffect.SetLightData(light.LightPos, light.LightColor, range);

		m_LocalLightEffect.Apply(m_ImmediateContext.Get());
		m_LocalLightEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

		m_ImmediateContext->DrawIndexed(m_SphereMeshIndexCount, 0, 0);
		
	}
	ResetStates();
	//---------------------------------------------------------------------------------//

	m_ImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	Present();
}

void DemoScene::ResetStates()
{
	m_ImmediateContext->RSSetState(nullptr);
	m_ImmediateContext->OMSetBlendState(nullptr, NULL, 0xffffffff);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);
}


void DemoScene::HammersleyBlockSetup()
{
	m_HammersleyData.N = 40; 
	m_HammersleyData.Values.resize(m_HammersleyData.N * 2);

	int kk;
	int pos = 0;
	for (int k = 0; k < m_HammersleyData.N; k++) 
	{
		kk = k;
		float u = 0.0f;
		for (float p = 0.5f; kk; p *= 0.5f, kk >>= 1)
		{
			if (kk & 1)
				u += p;
		}
		float v = (k + 0.5) / m_HammersleyData.N;
		m_HammersleyData.Values[pos++] = u;
		m_HammersleyData.Values[pos++] = v;
	}

}

void DemoScene::ComputeShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&m_DirLight.Direction);
	XMVECTOR lightPos = -2.0f * m_SceneBounds.Radius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&m_SceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - m_SceneBounds.Radius;
	float b = sphereCenterLS.y - m_SceneBounds.Radius;
	float n = sphereCenterLS.z - m_SceneBounds.Radius;
	float r = sphereCenterLS.x + m_SceneBounds.Radius;
	float t = sphereCenterLS.y + m_SceneBounds.Radius;
	float f = sphereCenterLS.z + m_SceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V * P * T;

	XMStoreFloat4x4(&m_LightView, V);
	XMStoreFloat4x4(&m_LightProj, P);
	XMStoreFloat4x4(&m_ShadowTransform, S);
}



void DemoScene::Present()
{
	static bool bVSync = true;
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Checkbox("VSync:", &bVSync);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
		ImGui::EndMainMenuBar();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (bVSync)
		m_SwapChain->Present(1, 0);
	else
		m_SwapChain->Present(0, 0);
}


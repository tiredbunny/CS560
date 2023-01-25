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

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;


extern DX::StepTimer g_Timer;

//These headers may not be present, so you may need to build the project once
namespace
{
	#include "Shaders\Compiled\BasicVS.h"
	#include "Shaders\Compiled\BasicSkinnedVS.h"
}

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_DrawableGrid = std::make_unique<Drawable>();
	m_DrawableSphere = std::make_unique<Drawable>();

	
	
	//Setup DirectionalLight
	m_DirLight.SetDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	//Setup Pointlight
	m_PointLight.Position = XMFLOAT3(0.0f, 2.0f, -2.0f);
	//Setup Spotlight
	m_SpotLight.Position = XMFLOAT3(-6.0f, 4.0f, 6.0f);
	m_SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.SpotPower = 2.0f;

	//Setup some material properties other than default

	m_DrawableGrid->Material.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_DrawableGrid->Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	m_DrawableSphere->Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_DrawableSphere->Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	m_Camera.SetPosition(-2.0f, 5.0f, -20.0f);
}

bool DemoScene::CreateDeviceDependentResources()
{

	m_Sky.Create(m_Device.Get(), L"Textures\\desertcube1024.dds", 3000.0f);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosNormalTex;
	DX::ThrowIfFailed
	(
		m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements,
		GeometricPrimitive::VertexType::InputElementCount,
		g_BasicVS, sizeof(g_BasicVS),
		layoutPosNormalTex.ReleaseAndGetAddressOf())
	);

	m_BasicEffect.Create(m_Device.Get(), layoutPosNormalTex);


	m_PrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_ImmediateContext.Get());
	m_DebugBasicEffect = std::make_unique<BasicEffect>(m_Device.Get());
	m_DebugBasicEffect->SetVertexColorEnabled(true);

	DX::ThrowIfFailed
	(
		CreateInputLayoutFromEffect<VertexPositionColor>(m_Device.Get(), m_DebugBasicEffect.get(), &m_DebugBasicEffectInputLayout)
	);

	m_CommonStates = std::make_unique<CommonStates>(m_Device.Get());

#pragma region Load Textures
	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\mipmaps.dds", 0, m_DrawableGrid->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableSphere->TextureSRV.ReleaseAndGetAddressOf()))
		return false;
#pragma endregion

#pragma region States Creation
	CD3D11_DEFAULT d3dDefault;
	{
		CD3D11_RASTERIZER_DESC desc(d3dDefault);
		desc.FrontCounterClockwise = TRUE;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSFrontCounterCW.ReleaseAndGetAddressOf()))
			return false;
	}
#pragma endregion

	CreateBuffers();

	return true;
}

void DemoScene::CreateBuffers()
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;
	
	Helpers::CreateGrid(vertices, indices, 100, 100);
	m_DrawableGrid->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateSphere(vertices, indices, 0.25f, 16, false);
	m_DrawableSphere->Create(m_Device.Get(), vertices, indices);

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
	

	if (GetAsyncKeyState('W') & 0x8000)
		m_Camera.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		m_Camera.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		m_Camera.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		m_Camera.Strafe(10.0f * dt);

	m_Camera.UpdateViewMatrix();


	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(20.0f, 20.0f, 0.0f));

	m_DrawableSphere->WorldTransform = Helpers::XMMatrixToStorage(
		XMMatrixTranslation(0.0f, -3.0f, 0.0f)
	);

	m_BasicEffect.SetEyePosition(m_Camera.GetPosition());
	m_BasicEffect.SetDirectionalLight(m_DirLight);
	m_BasicEffect.SetPointLight(m_PointLight);
	m_BasicEffect.SetSpotLight(m_SpotLight);
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());


#pragma region ImGui Widgets

	if (ImGui::Begin("Scene"))
	{
		if (ImGui::CollapsingHeader("Screen Resolution"))
		{
			//input of client size area
			static int inputWidth = m_ClientWidth;
			static int inputHeight = m_ClientHeight;

			ImGui::InputInt("Width", &inputWidth, 100, 200);
			ImGui::InputInt("Height", &inputHeight, 100, 200);

			if (ImGui::Button("Apply"))
			{
				//Should probably clamp it on min/max values but works for now
				if (inputWidth < MIN_WIDTH || inputHeight < MIN_HEIGHT ||
					inputWidth > MAX_WIDTH || inputHeight > MAX_HEIGHT)
				{
					inputWidth = MIN_WIDTH;
					inputHeight = MIN_HEIGHT;
				}

				if (inputWidth == m_ClientWidth && inputHeight == m_ClientHeight)
				{
					ImGui::End();
					return;
				}

				//Calculate window size from client size
				RECT wr = { 0, 0, inputWidth, inputHeight };
				AdjustWindowRect(&wr, g_WindowStyle, 0);

				UINT windowWidth = static_cast<UINT>(wr.right - wr.left);
				UINT windowHeight = static_cast<UINT>(wr.bottom - wr.top);

				//Resize window area
				assert(SetWindowPos(m_MainWindow, 0, 0, 0, windowWidth, windowHeight,
					SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER) != 0);

				//Resize color & depth buffers
				Super::OnResize();

			}
		}

		static XMFLOAT3 dirLightVec = m_DirLight.Direction;
		if (ImGui::CollapsingHeader("Lights"))
		{
			if (ImGui::TreeNode("Directional Light"))
			{
				static bool isActive = true;
				static XMFLOAT3 oldDirection;
				static XMFLOAT4 oldAmbient;
				if (ImGui::Checkbox("Is Active##1", &isActive))
				{
					if (!isActive)
					{
						oldDirection = dirLightVec;
						oldAmbient = m_DirLight.Ambient;

						//setting DirLightVector to zero-vector only zeros out diffuse & specular color
						dirLightVec = XMFLOAT3(0.0f, 0.0f, 0.0f);
						//we still need to zero out ambient color manually
						m_DirLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
					}
					else
					{
						m_DirLight.Ambient = oldAmbient;
						dirLightVec = oldDirection;
					}
				}

				ImGui::ColorEdit4("Ambient##1", reinterpret_cast<float*>(&m_DirLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##1", reinterpret_cast<float*>(&m_DirLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##1", reinterpret_cast<float*>(&m_DirLight.Specular), 2);

				ImGui::InputFloat3("Direction##1", reinterpret_cast<float*>(&dirLightVec), 2);
				m_DirLight.SetDirection(dirLightVec);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Point Light"))
			{
				static bool isActive = true;
				static float oldRange;
				if (ImGui::Checkbox("Is Active##2", &isActive))
				{
					if (!isActive)
					{
						oldRange = m_PointLight.Range;
						m_PointLight.Range = 0;
					}
					else
						m_PointLight.Range = oldRange;
				}

				ImGui::ColorEdit4("Ambient##2", reinterpret_cast<float*>(&m_PointLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##2", reinterpret_cast<float*>(&m_PointLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##2", reinterpret_cast<float*>(&m_PointLight.Specular), 2);
				ImGui::DragFloat3("Position##1", reinterpret_cast<float*>(&m_PointLight.Position));
				ImGui::InputFloat("Range##1", &m_PointLight.Range, 2);
				ImGui::InputFloat3("Attenuation##1", reinterpret_cast<float*>(&m_PointLight.Attenuation), 2);


				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Spot Light"))
			{
				static bool isActive = true;
				static float oldRange;
				if (ImGui::Checkbox("Is Active##3", &isActive))
				{
					if (!isActive)
					{
						oldRange = m_SpotLight.Range;
						m_SpotLight.Range = 0;
					}
					else
						m_SpotLight.Range = oldRange;
				}

				ImGui::ColorEdit4("Ambient##5", reinterpret_cast<float*>(&m_SpotLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##5", reinterpret_cast<float*>(&m_SpotLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##5", reinterpret_cast<float*>(&m_SpotLight.Specular), 2);
				ImGui::DragFloat3("Position##2", reinterpret_cast<float*>(&m_SpotLight.Position));
				ImGui::InputFloat("Range##2", reinterpret_cast<float*>(&m_SpotLight.Range), 2);
				ImGui::InputFloat("SpotPower", &m_SpotLight.SpotPower, 2);
				ImGui::InputFloat3("Direction##2", reinterpret_cast<float*>(&m_SpotLight.Direction), 2);
				ImGui::InputFloat3("Attenuation##2", reinterpret_cast<float*>(&m_SpotLight.Attenuation), 2);

				ImGui::TreePop();
			}
		}

		if (ImGui::CollapsingHeader("Object Materials"))
		{

			if (ImGui::TreeNode("Grid"))
			{
				ImGui::InputFloat4("Ambient##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Ambient), 2);
				ImGui::InputFloat4("Diffuse##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Diffuse), 2);
				ImGui::InputFloat4("Specular##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Specular), 2);

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}


#pragma endregion
}


void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::Black;

	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<float*>(&backBufferColor));
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DemoScene::PrepareForRendering()
{
	m_BasicEffect.Bind(m_ImmediateContext.Get());
	m_BasicEffect.SetSampler(m_ImmediateContext.Get(), m_CommonStates->AnisotropicWrap());

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void DemoScene::DrawScene()
{
	Clear();
	PrepareForRendering();

	XMMATRIX viewProj = m_Camera.GetView() * m_Camera.GetProj();
	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;


	//====================================== static objects ======================================//

	static Drawable* drawables[] = { m_DrawableGrid.get() };
	for (auto const& it : drawables)
	{
		FillBasicEffect(it);

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	m_ImmediateContext->RSSetState(nullptr);

	//====================================== Sky/background ======================================================//

	m_Sky.Draw(m_ImmediateContext.Get(), m_CommonStates->LinearWrap(), m_CommonStates->CullNone(),
		m_Camera.GetPosition3f(), viewProj, m_ClientWidth, m_ClientHeight,
		true, XMFLOAT4(0, 0, 0, 0), XMFLOAT4(1, 1, 1, 1));

	//============================================================================================================//

	m_ImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);
	m_ImmediateContext->RSSetState(nullptr);

	ResetStates();
	Present();
}

void DemoScene::FillBasicEffect(Drawable* drawable)
{
	XMMATRIX viewProj = m_Camera.GetView() * m_Camera.GetProj();

	m_BasicEffect.SetWorld(drawable->GetWorld());
	m_BasicEffect.SetWorldViewProj(drawable->GetWorld() * viewProj);
	m_BasicEffect.SetTextureTransform(XMLoadFloat4x4(&drawable->TextureTransform));
	m_BasicEffect.SetMaterial(drawable->Material);
	m_BasicEffect.SetTexture(m_ImmediateContext.Get(), drawable->TextureSRV.Get());

	m_BasicEffect.Apply(m_ImmediateContext.Get());
}

void DemoScene::ResetStates()
{
	m_ImmediateContext->RSSetState(nullptr);
	m_ImmediateContext->OMSetBlendState(nullptr, NULL, 0xffffffff);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);
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


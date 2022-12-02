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
using namespace SimpleMath;

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


	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosNormalTexSkinned;
	DX::ThrowIfFailed
	(
		m_Device->CreateInputLayout(SkinnedVertex::InputElements,
		SkinnedVertex::ElementCount,
		g_SkinnedVS, sizeof(g_SkinnedVS),
		layoutPosNormalTexSkinned.ReleaseAndGetAddressOf())
	);

	m_SkinnedEffect.Create(m_Device.Get(), layoutPosNormalTexSkinned);
	m_SkinnedModel = std::make_unique<SkinnedModel>(m_Device.Get(), "Models\\soldier.m3d", L"Textures\\");
	m_SkinnedModelInstance.Model = m_SkinnedModel.get();
	m_SkinnedModelInstance.TimePos = 0.0f;
	m_SkinnedModelInstance.ClipName = "Take1";
	m_SkinnedModelInstance.FinalTransforms.resize(m_SkinnedModel->SkinnedData.BoneCount());
	m_SkinnedModelInstance.BonePositions.resize(m_SkinnedModel->SkinnedData.BoneCount());


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

	m_SkinnedEffect.SetEyePosition(m_Camera.GetPosition());
	m_SkinnedEffect.SetDirectionalLight(m_DirLight);
	m_SkinnedEffect.SetPointLight(m_PointLight);
	m_SkinnedEffect.SetSpotLight(m_SpotLight);
	m_SkinnedEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());


	m_SkinnedModelInstance.Update(dt);

	XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, 0.05f);
	XMMATRIX modelOffset = g_Path->Update(timer);

	XMStoreFloat4x4(&m_SkinnedModelInstance.World, modelScale * modelOffset);

	for (int i = 0; i < m_SkinnedModelInstance.BonePositions.size(); ++i)
	{
		XMVECTOR bone = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i]);
		bone = XMVector3TransformCoord(bone, XMLoadFloat4x4(&m_SkinnedModelInstance.World));

		XMStoreFloat4(&m_SkinnedModelInstance.BonePositions[i], bone);
	}


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
					return;

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

	ImGui::Text("deltaTime: %f", dt);


#pragma endregion
}


void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::Black;
	
	if (ImGui::Begin("Scene"))
	{
		ImGui::ColorEdit4("background", reinterpret_cast<float*>(&backBufferColor));
		ImGui::End();
	}

	if (ImGui::Begin("Usage Instruction"))
	{
		ImGui::Text("Use WASD keys on Keyboard for movement.\nHold Right Click on Mouse to look around.");
		ImGui::End();
	}

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

	static bool drawSkinnedModel = true;
	static bool drawSkeleton = true;
	static bool drawPathCurve = true;
	
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

	//control points
	for (int i = 1; i < g_Path->m_StartingPoints.size() - 1; ++i)
	{
		auto& const point = g_Path->m_StartingPoints[i];

		XMMATRIX world = XMMatrixTranslation(point.x, point.y, point.z);

		m_BasicEffect.SetWorld(world);
		m_BasicEffect.SetWorldViewProj(world * viewProj);
		m_BasicEffect.SetTextureTransform(XMMatrixIdentity());
		m_BasicEffect.SetMaterial(m_DrawableSphere->Material);
		m_BasicEffect.SetTexture(m_ImmediateContext.Get(), m_DrawableSphere->TextureSRV.Get());

		m_BasicEffect.Apply(m_ImmediateContext.Get());

		m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableSphere->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(m_DrawableSphere->IndexBuffer.Get(), m_DrawableSphere->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(m_DrawableSphere->IndexCount, 0, 0);
	}

	//Joint spheres
	for (int i = 0; i < m_SkinnedModelInstance.BonePositions.size(); ++i)
	{
		if (!(!drawSkinnedModel && drawSkeleton))
			break;

		auto jointPos = m_SkinnedModelInstance.BonePositions[i];

		XMMATRIX world = XMMatrixScaling(0.3f, 0.3f, 0.3f) * 
			XMMatrixTranslation(jointPos.x, jointPos.y, jointPos.z);
			
		m_BasicEffect.SetWorld(world);
		m_BasicEffect.SetWorldViewProj(world * viewProj);

		m_BasicEffect.Apply(m_ImmediateContext.Get());

		m_ImmediateContext->DrawIndexed(m_DrawableSphere->IndexCount, 0, 0);
	}
	
	//====================================== Skinned Model  ======================================//
	
	if (ImGui::Begin("Scene"))
	{
		ImGui::Checkbox("Draw Skinned Model", &drawSkinnedModel);

		ImGui::End();
	}

	if (drawSkinnedModel)
	{
		m_ImmediateContext->RSSetState(m_RSFrontCounterCW.Get());
		m_SkinnedEffect.Bind(m_ImmediateContext.Get());
		m_SkinnedEffect.SetSampler(m_ImmediateContext.Get(), m_CommonStates->AnisotropicWrap());

		m_SkinnedEffect.SetWorld(XMLoadFloat4x4(&m_SkinnedModelInstance.World));
		m_SkinnedEffect.SetWorldViewProj(XMLoadFloat4x4(&m_SkinnedModelInstance.World) * viewProj);
		m_SkinnedEffect.SetTextureTransform(XMMatrixIdentity());
		m_SkinnedEffect.SetBoneTransforms(m_SkinnedModelInstance.FinalTransforms.data(), m_SkinnedModelInstance.FinalTransforms.size());

		for (UINT subset = 0; subset < m_SkinnedModelInstance.Model->SubsetCount; ++subset)
		{
			m_SkinnedEffect.SetMaterial(m_SkinnedModelInstance.Model->Mat[subset]);
			m_SkinnedEffect.SetTexture(m_ImmediateContext.Get(), m_SkinnedModelInstance.Model->DiffuseMapSRV[subset]);

			m_SkinnedEffect.Apply(m_ImmediateContext.Get());
			m_SkinnedModelInstance.Model->ModelMesh.Draw(m_ImmediateContext.Get(), subset);

		}
	}
	


	//====================================== Sky/background ======================================================//

	m_Sky.Draw(m_ImmediateContext.Get(), m_CommonStates->LinearWrap(), m_CommonStates->CullNone(),
		m_Camera.GetPosition3f(), viewProj, m_ClientWidth, m_ClientHeight,
		true, XMFLOAT4(0, 0, 0, 0), XMFLOAT4(1, 1, 1, 1));

	//====================================== Skeleton line & Path curve drawing ======================================//


	if (ImGui::Begin("Scene"))
	{
		ImGui::Checkbox("Draw Skeleton", &drawSkeleton);
		ImGui::Checkbox("Draw Path", &drawPathCurve);

		ImGui::End();
	}

	m_ImmediateContext->OMSetBlendState(m_CommonStates->Opaque(), nullptr, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(m_CommonStates->DepthNone(), 0);
	m_ImmediateContext->RSSetState(m_CommonStates->CullNone());

	m_DebugBasicEffect->SetView(m_Camera.GetView());
	m_DebugBasicEffect->SetProjection(m_Camera.GetProj());
	m_DebugBasicEffect->Apply(m_ImmediateContext.Get());

	m_ImmediateContext->IASetInputLayout(m_DebugBasicEffectInputLayout.Get());

	m_PrimitiveBatch->Begin();

	if (drawSkeleton)
	{
		for (int i = 0; i < m_SkinnedModelInstance.BonePositions.size() - 1; ++i)
		{
			XMVECTOR origin = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i]);
			XMVECTOR direction = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i + 1]) - origin;

			if (m_SkinnedModelInstance.Model->SkinnedData.mBoneHierarchy[i + 1] == i)
				DX::DrawRay(m_PrimitiveBatch.get(), origin, direction, false, Colors::Red);
		}
	}

	if (drawPathCurve)
	{
		for (int i = 0; i < g_Path->m_PlotPoints.size() - 1; ++i)
		{
			auto origin = g_Path->m_PlotPoints[i];
			auto direction = g_Path->m_PlotPoints[i + 1] - origin;

			DX::DrawRay(m_PrimitiveBatch.get(), origin, direction, false, Colors::Red);
		}
	}

	m_PrimitiveBatch->End();


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


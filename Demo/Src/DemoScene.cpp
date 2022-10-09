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
#include "MathHelper.h"
#include <VertexTypes.h>
#include <DirectXHelpers.h>
#include <DebugDraw.h>

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;

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
	m_DrawableBox = std::make_unique<Drawable>();
	m_DrawableSphere = std::make_unique<Drawable>();
	m_DrawableTorus = std::make_unique<Drawable>();
	m_DrawableTeapot = std::make_unique<Drawable>();
	m_DrawableGrid = std::make_unique<Drawable>();

	//Setup DirectionalLight
	m_DirLight.SetDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	//Setup Pointlight
	m_PointLight.Position = XMFLOAT3(0.0f, 2.0f, -2.0f);
	//Setup Spotlight
	m_SpotLight.Position = XMFLOAT3(-6.0f, 4.0f, 6.0f);
	m_SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.SpotPower = 2.0f;

	//Setup some material properties other than default
	m_DrawableBox->Material.Ambient = XMFLOAT4(0.35f, 0.35f, 0.35f, 1.0f);
	m_DrawableSphere->Material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DrawableSphere->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 64.0f);
	m_DrawableTorus->Material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	m_DrawableTeapot->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	m_DrawableGrid->Material.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_DrawableGrid->Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

bool DemoScene::CreateDeviceDependentResources()
{
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
	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\crate.dds", 0, m_DrawableBox->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\mipmaps.dds", 0, m_DrawableGrid->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableSphere->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\rock.jpg", 0, m_DrawableTorus->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\flooring.png", 0, m_DrawableTeapot->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

#pragma endregion

#pragma region States Creation
	CD3D11_DEFAULT d3dDefault;
	{
		CD3D11_SAMPLER_DESC desc(d3dDefault);
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MaxAnisotropy = 8;

		if FAILED(m_Device->CreateSamplerState(&desc, m_SamplerAnisotropic.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_BLEND_DESC desc(d3dDefault);
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSTransparent.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_BLEND_DESC(d3dDefault);
		desc.RenderTarget[0].RenderTargetWriteMask = NULL;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSNoColorWrite.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_BLEND_DESC(d3dDefault);
		desc.AlphaToCoverageEnable = TRUE;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSAlphaToCoverage.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_RASTERIZER_DESC desc(d3dDefault);
		desc.CullMode = D3D11_CULL_NONE;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSCullNone.ReleaseAndGetAddressOf()))
			return false;
		
		desc = CD3D11_RASTERIZER_DESC(d3dDefault);
		desc.FillMode = D3D11_FILL_WIREFRAME;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSWireframe.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_RASTERIZER_DESC(d3dDefault);
		desc.FrontCounterClockwise = TRUE;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSFrontCounterCW.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_DEPTH_STENCIL_DESC desc(d3dDefault);
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSNoDepthWrite.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSMarkPixels.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSDrawMarkedOnly.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSNoDoubleBlend.ReleaseAndGetAddressOf()))
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
	
	GeometricPrimitive::CreateSphere(vertices, indices, 3.0f, 16, false);
	m_DrawableSphere->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(2.5f, 2.5f, 2.5f), false);
	m_DrawableBox->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateTorus(vertices, indices, 3.0f, 1.0f, 32, false);
	m_DrawableTorus->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateTeapot(vertices, indices, 3.0f, 8, false);
	m_DrawableTeapot->Create(m_Device.Get(), vertices, indices);

	Helpers::CreateGrid(vertices, indices, 100, 100);
	m_DrawableGrid->Create(m_Device.Get(), vertices, indices);
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

void DemoScene::UpdateScene(float dt)
{
	Super::ImGui_NewFrame();

	//values updated by ImGui widgets
	static float cameraAngle = 0.0f;
	static XMVECTOR initEyePos = XMVectorSet(0.0f, 7.0f, -20.0f, 1.0f);
	static XMVECTOR focus = XMVectorSet(0, 0, 0, 1);

	//build projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_CameraProjection, proj);

	//build view matrix
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR eyePosition = XMVector3Transform(initEyePos, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));
	XMMATRIX view = XMMatrixLookAtLH(eyePosition, focus, up);
	XMStoreFloat4x4(&m_CameraView, view);

	static float angle = 0.0f;
	angle += 45.0f * dt;
	
	XMMATRIX rotateY = XMMatrixRotationY(XMConvertToRadians(angle));

	m_DrawableBox->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-2, 2, 0));
	m_DrawableTeapot->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(2, 2, 10));
	m_DrawableSphere->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(3, 2, 0));
	m_DrawableTorus->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-4, 2, 6));

	XMStoreFloat4x4(&m_DrawableSphere->TextureTransform, XMMatrixScaling(2.0f, 2.0f, 0.0f));
	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(20.0f, 20.0f, 0.0f));


	m_BasicEffect.SetEyePosition(eyePosition);
	m_BasicEffect.SetDirectionalLight(m_DirLight);
	m_BasicEffect.SetPointLight(m_PointLight);
	m_BasicEffect.SetSpotLight(m_SpotLight);
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

	m_SkinnedEffect.SetEyePosition(eyePosition);
	m_SkinnedEffect.SetDirectionalLight(m_DirLight);
	m_SkinnedEffect.SetPointLight(m_PointLight);
	m_SkinnedEffect.SetSpotLight(m_SpotLight);
	m_SkinnedEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());


	m_SkinnedModelInstance.Update(dt);

	static XMMATRIX modelScale = XMMatrixScaling(0.1f, 0.1f, 0.1f);
	static XMMATRIX modelRot = XMMatrixIdentity();
	static XMMATRIX modelOffset = XMMatrixTranslation(-2.0f, 0.0f, -7.0f);
	XMStoreFloat4x4(&m_SkinnedModelInstance.World, modelScale * modelRot * modelOffset);

	for (int i = 0; i < m_SkinnedModelInstance.BonePositions.size(); ++i)
	{
		XMVECTOR bone = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i]);
		bone = XMVector3TransformCoord(bone, XMLoadFloat4x4(&m_SkinnedModelInstance.World));

		XMStoreFloat4(&m_SkinnedModelInstance.BonePositions[i], bone);
	}


#pragma region ImGui Widgets
	if (!ImGui::Begin("Scene"))
	{
		ImGui::End();
		return;
	}

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
				SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER) != 0);
			
			//Resize color & depth buffers
			Super::OnResize();

		}
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::DragFloat3("EyePos", reinterpret_cast<float*>(&initEyePos), 1.0f, -100.0f, 100.0f, "%.2f");
		ImGui::DragFloat3("Focus", reinterpret_cast<float*>(&focus), 1.0f, -100.0f, 100.0f), "%.2f";
		
		if (XMVector3Equal(initEyePos, XMVectorZero()))
			initEyePos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

		ImGui::SliderFloat("Rotate-Y", &cameraAngle, 0.0f, 360.0f);
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
		if (ImGui::TreeNode("Sphere"))
		{
			ImGui::InputFloat4("Ambient##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Specular), 2);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Box"))
		{
			ImGui::InputFloat4("Ambient##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Specular), 2);

			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Torus"))
		{
			ImGui::InputFloat4("Ambient##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Specular), 2);

			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Teapot"))
		{
			ImGui::InputFloat4("Ambient##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Specular), 2);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Grid"))
		{
			ImGui::InputFloat4("Ambient##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Specular), 2);

			ImGui::TreePop();
		}
	}

	ImGui::Text("deltaTime: %f", dt);

	ImGui::End();

#pragma endregion
}


void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::AntiqueWhite;
	ImGui::PushItemWidth(ImGui::GetColumnWidth() * 0.5f);
	ImGui::ColorEdit4("clear color", reinterpret_cast<float*>(&backBufferColor));
	ImGui::PopItemWidth();

	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<float*>(&backBufferColor));
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DemoScene::PrepareForRendering()
{
	m_BasicEffect.Bind(m_ImmediateContext.Get());
	m_BasicEffect.SetSampler(m_ImmediateContext.Get(), m_SamplerAnisotropic.Get());

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void DemoScene::DrawScene()
{
	Clear();
	PrepareForRendering();

	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);
	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	static Drawable* drawables[] = { m_DrawableGrid.get(), m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };
	for (auto const& it : drawables)
	{
		FillBasicEffect(it);

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	
	// Instance 1
	m_ImmediateContext->RSSetState(m_RSFrontCounterCW.Get());
	m_SkinnedEffect.Bind(m_ImmediateContext.Get());
	m_SkinnedEffect.SetSampler(m_ImmediateContext.Get(), m_SamplerAnisotropic.Get());

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

	//====================================== debug drawing ======================================//

	m_ImmediateContext->OMSetBlendState(m_CommonStates->Opaque(), nullptr, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(m_CommonStates->DepthNone(), 0);
	m_ImmediateContext->RSSetState(m_CommonStates->CullNone());

	m_DebugBasicEffect->SetView(XMLoadFloat4x4(&m_CameraView));
	m_DebugBasicEffect->SetProjection(XMLoadFloat4x4(&m_CameraProjection));
	m_DebugBasicEffect->Apply(m_ImmediateContext.Get());

	m_ImmediateContext->IASetInputLayout(m_DebugBasicEffectInputLayout.Get());

	m_PrimitiveBatch->Begin();


	for (int i = 0; i < m_SkinnedModelInstance.BonePositions.size() - 1; ++i)
	{
		XMVECTOR origin = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i]);
		XMVECTOR direction = XMLoadFloat4(&m_SkinnedModelInstance.BonePositions[i + 1]) - origin;

		DX::DrawRay(m_PrimitiveBatch.get(), origin, direction, false, Colors::Red);

	}

	m_PrimitiveBatch->End();

	m_ImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);
	m_ImmediateContext->RSSetState(nullptr);

	
	ResetStates();
	Present();
}

void DemoScene::FillBasicEffect(Drawable* drawable)
{
	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);

	m_BasicEffect.SetWorld(drawable->GetWorld());
	m_BasicEffect.SetWorldViewProj(drawable->GetWorld() * viewProj);
	m_BasicEffect.SetTextureTransform(XMLoadFloat4x4(&drawable->TextureTransform));
	m_BasicEffect.SetMaterial(drawable->Material);
	m_BasicEffect.SetTexture(m_ImmediateContext.Get(), drawable->TextureSRV.Get());

	m_BasicEffect.Apply(m_ImmediateContext.Get());
}

void DemoScene::ResetStates()
{
	static bool bWireframe = false;
	ImGui::Checkbox("Wireframe", &bWireframe);
	
	if (bWireframe)
		m_ImmediateContext->RSSetState(m_RSWireframe.Get());
	else
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


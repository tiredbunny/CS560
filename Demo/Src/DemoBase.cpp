#include "pch.h"
#include "DemoBase.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace Microsoft::WRL;


DemoBase::DemoBase(const HWND& hwnd) :
	m_MainWindow(hwnd), 
	m_MSAAQuality(0),
	M_MSAASampleCount(4),
	m_BackBufferFormat(DXGI_FORMAT_R8G8B8A8_UNORM),
	m_DepthStencilBufferFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
	m_DepthStencilViewFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
{
	assert(hwnd != nullptr);
	 
	m_ScreenViewport = { };
	
	UpdateClientSizeVars();
}

void DemoBase::UpdateClientSizeVars()
{
	RECT rect;
	GetClientRect(m_MainWindow, &rect);
	m_ClientWidth = static_cast<UINT>(rect.right - rect.left);
	m_ClientHeight = static_cast<UINT>(rect.bottom - rect.top);
}

DemoBase::~DemoBase()
{
	ImGui_Destroy();
}

void DemoBase::ImGui_Destroy()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
void DemoBase::ImGui_Init()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_MainWindow);
	ImGui_ImplDX11_Init(m_Device.Get(), m_ImmediateContext.Get());
}

void DemoBase::ImGui_NewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

bool DemoBase::Initialize()
{

	UINT flags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		m_Device.ReleaseAndGetAddressOf(),
		&featureLevel,
		m_ImmediateContext.ReleaseAndGetAddressOf());

	if (FAILED(hr))
		return false;

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
		return false;

	DXGI_SWAP_CHAIN_DESC sd;
	
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator= 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	DX::ThrowIfFailed(m_Device->CheckMultisampleQualityLevels(m_BackBufferFormat, M_MSAASampleCount, &m_MSAAQuality));
	assert(m_MSAAQuality > 0);

	sd.SampleDesc.Count = M_MSAASampleCount;
	sd.SampleDesc.Quality = m_MSAAQuality - 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = m_MainWindow;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	ComPtr<IDXGIDevice> dxgiDevice;
	if FAILED(m_Device.As(&dxgiDevice))
		return false;

	ComPtr<IDXGIAdapter> dxgiAdapter;
	if FAILED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()))
		return false;

	ComPtr<IDXGIFactory1> dxgiFactory;
	if FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), &dxgiFactory))
		return false;

	if FAILED(dxgiFactory->CreateSwapChain(m_Device.Get(), &sd, m_SwapChain.ReleaseAndGetAddressOf()))
		return false;

	//disable ALT-enter fullscreen
	dxgiFactory->MakeWindowAssociation(m_MainWindow, DXGI_MWA_NO_WINDOW_CHANGES);
	
	OnResize();

	return true;
}

void DemoBase::OnResize()
{
	UpdateClientSizeVars();

	m_RenderTargetView.Reset();
	m_DepthStencilView.Reset();
	m_DepthStencilBuffer.Reset();

	DX::ThrowIfFailed(m_SwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, m_BackBufferFormat, 0));
	
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	DX::ThrowIfFailed(m_Device->CreateRenderTargetView(backBuffer.Get(), 0, m_RenderTargetView.ReleaseAndGetAddressOf()));

	backBuffer.Reset();

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Width = m_ClientWidth;
	depthStencilDesc.Height = m_ClientHeight;
	depthStencilDesc.Format = m_DepthStencilBufferFormat;
	depthStencilDesc.SampleDesc.Count = M_MSAASampleCount;
	depthStencilDesc.SampleDesc.Quality = m_MSAAQuality - 1;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	DX::ThrowIfFailed(m_Device->CreateTexture2D(&depthStencilDesc, 0, m_DepthStencilBuffer.ReleaseAndGetAddressOf()));


	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(
		m_DepthStencilBuffer.Get(), 
		M_MSAASampleCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D, 
		m_DepthStencilViewFormat);

	DX::ThrowIfFailed(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, m_DepthStencilView.ReleaseAndGetAddressOf()));

	m_ImmediateContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

	m_ScreenViewport.TopLeftX = 0.0f;
	m_ScreenViewport.TopLeftY = 0.0f;
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);

	m_ImmediateContext->RSSetViewports(1, &m_ScreenViewport);

}


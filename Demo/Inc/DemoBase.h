#pragma once

#include <d3d11.h>
#include <wrl/client.h>

constexpr auto MIN_WIDTH = 1280;
constexpr auto MIN_HEIGHT = 720;

constexpr auto MAX_WIDTH = 3840;
constexpr auto MAX_HEIGHT = 2160;


//use constructor for initializing variables
//use Initialize() for creating resources

class DemoBase
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	D3D11_VIEWPORT m_ScreenViewport;
	
	HWND m_MainWindow;

	UINT m_ClientWidth;
	UINT m_ClientHeight;
	UINT m_MSAAQuality;
	UINT M_MSAASampleCount;

	DXGI_FORMAT m_BackBufferFormat;
	DXGI_FORMAT m_DepthStencilBufferFormat;
	DXGI_FORMAT m_DepthStencilViewFormat;
	
public:
	virtual bool Initialize();
	virtual void OnResize();

	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;

protected:
	DemoBase(const HWND& hwnd);
	virtual ~DemoBase();

	void ImGui_NewFrame();
	void ImGui_Init();
	void ImGui_Destroy();
	void UpdateClientSizeVars();
};
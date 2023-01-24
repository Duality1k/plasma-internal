#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

using namespace utils;

#define OBS_PRESENT 0x32E6C // OBS module .rdata pRealPresent offset
#define OBS_RESIZEBUFFER 0x32E68 // OBS module .rdata pRealResizeBuffer offset

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace render
{
	typedef HRESULT(WINAPI* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);




	class OBS
	{
	private:
	public:
		std::uintptr_t ModuleBase = NULL;
		ID3D11Device* pDevice = nullptr;
		// ID3D11DeviceContext* g_pDeviceContext = nullptr;
		ID3D11DeviceContext* pContext = nullptr;
		// ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
		ID3D11RenderTargetView* pRTView = nullptr;
		Present OriginalPresent = nullptr;
		ResizeBuffers OriginalResizeBuffers;

		OBS();

		bool WaitForOBSModule();

		bool WaitForOBSPresent(Present pDetourPresent);

		bool HookPresent(std::function<bool()> render);
	};

	OBS* Obs = nullptr;
	std::function<bool()> Render;

	HWND Window = NULL;
	WNDPROC oWndProc = NULL;

	BOOL CALLBACK FindCurrentWindow(HWND hwnd, LPARAM lParam) {
		DWORD hwndProcessId = 0;
		const auto hwndThreadId = GetWindowThreadProcessId(hwnd, &hwndProcessId);
		if (hwndProcessId == GetCurrentProcessId() && hwndThreadId == GetWindowThreadProcessId(GetForegroundWindow(), NULL)) {
			Window = hwnd;
			return FALSE;
		}

		return TRUE;
	}

	LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
			return true;
		}
		return CallWindowProcA(oWndProc, hWnd, uMsg, wParam, lParam);
	}

	HRESULT STDMETHODCALLTYPE ResizeBuffersDetour(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		// Release the old render target view
		if (Obs->pRTView)
			Obs->pRTView->Release();

		// Call the original ResizeBuffers method
		HRESULT hr = Obs->OriginalResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		if (FAILED(hr))
			return hr;

		// Get the new render target view
		ID3D11Texture2D* pBackBuffer = nullptr;
		hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		hr = Obs->pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &Obs->pRTView);
		if (FAILED(hr))
		{
			pBackBuffer->Release();
			return hr;
		}
		pBackBuffer->Release();

		// Set the new render target view
		Obs->pContext->OMSetRenderTargets(1, &Obs->pRTView, nullptr);

		// Set the viewport for the new render target size
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)Width;
		vp.Height = (FLOAT)Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Obs->pContext->RSSetViewports(1, &vp);

		return hr;
	}

	HRESULT WINAPI PresentDetour(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {

		if (!Obs->pDevice)
		{
			pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Obs->pDevice);
			Obs->pDevice->GetImmediateContext(&Obs->pContext);

			DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
			pSwapChain->GetDesc(&SwapChainDesc);
			Window = SwapChainDesc.OutputWindow;

			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			Obs->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &Obs->pRTView);
			pBackBuffer->Release();

			EnumWindows(FindCurrentWindow, NULL);

			oWndProc = (WNDPROC)SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)WndProc);

			ImGui::CreateContext();
			ImGui_ImplWin32_Init(Window);
			ImGui_ImplDX11_Init(Obs->pDevice, Obs->pContext);

			// Get the address of the original ResizeBuffers method
			Obs->OriginalResizeBuffers = memory::read<ResizeBuffers>(Obs->ModuleBase + OBS_RESIZEBUFFER);
			// Replace the original ResizeBuffers method with our hooked version
			memory::write<ResizeBuffers>(Obs->ModuleBase + OBS_RESIZEBUFFER, &ResizeBuffersDetour);
			__Ok("ResizeBuffers detoured!");
		}

		if (Obs->pRTView == NULL)
		{
			ID3D11Texture2D* pBackBuffer = NULL;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			Obs->pDevice->CreateRenderTargetView(pBackBuffer, NULL, &Obs->pRTView);
			pBackBuffer->Release();
		}

		Render();

		return Obs->OriginalPresent(pSwapChain, SyncInterval, Flags);
	}

	OBS::OBS() {
		if (Obs == nullptr) Obs = this;
	}

	bool OBS::WaitForOBSModule() {
		while (!this->ModuleBase) {
			if (!(this->ModuleBase = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(xor ("graphics-hook32.dll"))))) {
				__Warn("Waiting for obs module to load...");
				Sleep(5000);
			}
		}

		return this->ModuleBase != NULL;
	}

	bool OBS::WaitForOBSPresent(Present pDetourPresent) {

		const auto rdataPresentAddress = this->ModuleBase + OBS_PRESENT;
		while (!memory::read<Present>(rdataPresentAddress)) {
			__Warn("Waiting for obs present to load...");
			Sleep(5000);
		}

		this->OriginalPresent = memory::read<Present>(rdataPresentAddress);
		memory::write<Present>(rdataPresentAddress, pDetourPresent);
		__Ok("pPresent Detoured");
		return true;
	}

	bool OBS::HookPresent(std::function<bool()> render) {

		Render = render;

		if (!this->WaitForOBSModule())
			return false;

		if (!this->WaitForOBSPresent(&PresentDetour))
			return false;
	}
}
#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#define OBS_PRESENT 0x32E6C // OBS module .rdata pPresent offset
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace render
{
	typedef HRESULT(WINAPI* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

	class OBS
	{
	private:
		std::uintptr_t moduleBaseAddress = NULL;
	public:
		ID3D11Device* pDevice = nullptr;
		ID3D11DeviceContext* pContext = nullptr;
		ID3D11RenderTargetView* pRTView = nullptr;
		Present OriginalPresent = nullptr;

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
		while (!this->moduleBaseAddress) {
			if (!(this->moduleBaseAddress = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(xor ("graphics-hook32.dll"))))) {
				__Warn("Waiting for obs module to load...");
				Sleep(5000);
			}
		}

		return this->moduleBaseAddress != NULL;
	}

	bool OBS::WaitForOBSPresent(Present pDetourPresent) {

		const auto rdataPresentAddress = this->moduleBaseAddress + OBS_PRESENT;
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
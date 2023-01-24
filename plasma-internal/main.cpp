#include <Windows.h>
#include <functional>
#include <memory>
#include <thread>

#include "config.h"
#include "utils/utils.h"
#include "render/render.hpp"
#include "roblox/roblox.h"

void MainThread() {
    const auto console = new Console();
    console->Attach();
    console->SetTitle(xor(L"Plasma"));

    Log::InitStdOutHandle();
    Log::SetLogLevel(LogLevel::Medium);

    __Ok("spy");
    //const auto spy = new rbx::triggers::Spy();
    __Ok("spy created");
    //spy->Initialize();
    __Ok("spy init");

    const auto obs = new render::OBS();
    obs->HookPresent([]() {
        if (GetAsyncKeyState(VK_INSERT) & 1)
        config::bShowMenu = !config::bShowMenu;

        ImGui::StyleColorsClassic();

        render::Obs->pContext->OMSetRenderTargets(1, &render::Obs->pRTView, NULL);

        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();

        if (config::bShowMenu)
        {
            __Warn("ImGui::Render");
            ImGui::Begin(xor ("Plasma"));

            ImGui::Button("gay retard", ImVec2{ 200, 50 });

            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        return true;
    });
}

bool _stdcall DllMain(HMODULE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
    DisableThreadLibraryCalls(hModule);

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        std::thread{ MainThread }.detach();

    return TRUE;
}


#include <Windows.h>
#include <functional>
#include <memory>
#include <thread>
#include <dxgi.h>
#include <d3d11.h>

#include "config.h"
#include "utils/utils.h"
#include "render/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "render/imgui/imgui_internal.h"
#include "render/imgui/imgui_impl_dx11.h"
#include "render/imgui/imgui_impl_win32.h"
#include "render/imgui/font.hpp"
#include "render/imgui/custom.hpp"
#include "render/render.hpp"
#include "roblox/roblox.h"
#include "roblox/executor.hpp"

#define DISABLE_REMOTE_SPY

void MainThread() {
    const auto console = new Console();
    console->Attach();
    console->SetTitle(xor(L"Plasma"));

    Log::InitStdOutHandle();
    Log::SetLogLevel(LogLevel::Full);

#ifndef DISABLE_REMOTE_SPY
    const auto spy = new rbx::triggers::Spy();
    spy->Initialize();
    __Ok("Remote SPY Initialized");
#endif

    const auto scheduler = new rbx::Scheduler();
    const auto job = scheduler->getJobByName(xor("WaitingHybridScriptsJob"));
    const auto scriptContext = memory::read<std::uintptr_t>(job + rbx::offsets::script_context);
    __Ok("ScriptContext: 0x%x", scriptContext);

    const auto obs = new render::OBS();
    obs->HookPresent([]() {
        if (GetAsyncKeyState(VK_INSERT) & 1)
            config::bShowMenu = !config::bShowMenu;

        if (!config::bLoadedFonts) {
            
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontFromMemoryTTF((void*)font_binary, sizeof(font_binary), 13.f);
            io.Fonts->AddFontFromMemoryTTF((void*)icons_binary, sizeof(icons_binary), 15.f);
            io.Fonts->AddFontFromMemoryTTF((void*)font_bold_binary, sizeof(font_bold_binary), 20.f);
            config::bLoadedFonts = TRUE;
        }

        render::Obs->pContext->OMSetRenderTargets(1, &render::Obs->pRTView, NULL);

        ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX11_NewFrame();
        ImGui::NewFrame();

        if (config::bShowMenu)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
            ImGui::Begin("menu", 0, ImGuiWindowFlags_NoDecoration);
            {
                ImGui::PopStyleVar();

                ImGuiWindow* window = ImGui::GetCurrentWindow();

                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetWindowPos();

                ImGui::SetWindowSize({ 650, 450 });

                static bool navbar_collapsed = true;
                static float navbar_width = 0.f; navbar_width = ImLerp(navbar_width, navbar_collapsed ? 0.f : 1.f, 0.04f);
                content_anim = ImLerp(content_anim, 1.f, 0.04f);

                {
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !navbar_collapsed);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, content_anim * (navbar_width > 0.005f ? 1 / navbar_width / 2 : 1.f));
                    ImGui::SetCursorPos({ 81, 25 });
                    ImGui::BeginGroup();
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
                    ImGui::Text("Page 1");
                    ImGui::PopFont();

                    ImGui::BeginChild("main", { window->Size.x - 81, window->Size.y - window->DC.CursorPos.y + window->Pos.y }, 0, ImGuiWindowFlags_NoBackground);
                    {
                        switch (tab) {
                        case 0: {
                            ImGui::BeginChild("subtabs", { ImGui::GetWindowWidth() - 30, 40 }, 1);
                            {
                                ImGui::SetCursorPos({ 16, 0 });

                                if (custom::subtab("Page 1##subtab", subtab == 0)) subtab = 0; ImGui::SameLine(0, 20);
                                if (custom::subtab("Page 2##subtab", subtab == 1)) subtab = 1; ImGui::SameLine(0, 20);
                                if (custom::subtab("Page 3##subtab", subtab == 2)) subtab = 2;
                            }
                            ImGui::EndChild();

                            ImGui::BeginChild("main_child", { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 55 }, 0, ImGuiWindowFlags_NoBackground);
                            {
                                ImGui::BeginGroup();
                                {
                                    custom::begin_child("Window 1", { (ImGui::GetWindowWidth() - 30) / 2 - ImGui::GetStyle().ItemSpacing.x / 2, 0 });
                                    {
                                        static bool enabled = true;
                                        static int slider = 0, combo = 0;
                                        static char text[64];
                                        static float col[4];

                                        ImGui::Checkbox("Enabled", &enabled);
                                        ImGui::SliderInt("Slider", &slider, 0, 100, "%d%%");
                                        ImGui::Combo("Type", &combo, "Type 1\0\rType 2\0\rType 3\0\0");
                                        ImGui::InputText("Text Field", text, 64);
                                        ImGui::ColorEdit4("Color Edit", col, ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayHex);
                                    }
                                    custom::end_child();

                                    custom::begin_child("Window 3", { (ImGui::GetWindowWidth() - 30) / 2 - ImGui::GetStyle().ItemSpacing.x / 2, 0 });
                                    {
                                        static bool enabled = true;
                                        static int slider = 0, combo = 0;
                                        static char text[64];
                                        static float col[4];

                                        ImGui::Checkbox("Enabled##2", &enabled);
                                        ImGui::SliderInt("Slider##2", &slider, 0, 100, "%d%%");
                                        ImGui::Combo("Type##2", &combo, "Type 1\0\rType 2\0\rType 3\0\0");
                                        ImGui::InputText("Text Field##2", text, 64);
                                        ImGui::ColorEdit4("Color Edit##2", col, ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayHex);
                                        ImGui::Button("Button##2");
                                    }
                                    custom::end_child();
                                }
                                ImGui::EndGroup();
                                ImGui::SameLine(0, 15);
                                ImGui::BeginGroup();
                                {
                                    custom::begin_child("Window 2", { (ImGui::GetWindowWidth() - 30) / 2 - ImGui::GetStyle().ItemSpacing.x / 2, 0 });
                                    {
                                        static bool enabled = true;
                                        static int slider = 0, combo = 0;
                                        static char text[64];
                                        static float col[4];

                                        ImGui::Checkbox("Enabled##3", &enabled);
                                        ImGui::SliderInt("Slider##3", &slider, 0, 100, "%d%%");
                                        ImGui::Combo("Type##3", &combo, "Type 1\0\rType 2\0\rType 3\0\0");
                                        ImGui::InputText("Text Field##3", text, 64);
                                        ImGui::ColorEdit4("Color Edit##3", col, ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayHex);
                                        ImGui::Button("Button##3");
                                    }
                                    custom::end_child();

                                    custom::begin_child("Window 4", { (ImGui::GetWindowWidth() - 30) / 2 - ImGui::GetStyle().ItemSpacing.x / 2, 0 });
                                    {
                                        static bool enabled = true;
                                        static int slider = 0, combo = 0;
                                        static char text[64];
                                        static float col[4];

                                        ImGui::Checkbox("Enabled##4", &enabled);
                                        ImGui::SliderInt("Slider##4", &slider, 0, 100, "%d%%");
                                        ImGui::Combo("Type##4", &combo, "Type 1\0\rType 2\0\rType 3\0\0");
                                        ImGui::InputText("Text Field##4", text, 64);
                                        ImGui::ColorEdit4("Color Edit##4", col, ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayHex);
                                    }
                                    custom::end_child();
                                }
                                ImGui::EndGroup();
                                ImGui::Spacing();
                            }
                            ImGui::EndChild();
                        } break;
                        }
                    }
                    ImGui::EndChild();

                    ImGui::EndGroup();
                    ImGui::PopStyleVar();
                    ImGui::PopItemFlag();
                }

                ImGui::SetCursorPos({ 0, 0 });

                ImGui::BeginChild("navbar", { 50 + 100 * navbar_width, window->Size.y }, 0, ImGuiWindowFlags_NoBackground);
                {
                    ImGui::GetWindowDrawList()->AddRectFilled(p, p + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_ChildBg), ImGui::GetStyle().WindowRounding, ImDrawFlags_RoundCornersLeft);
                    ImGui::GetWindowDrawList()->AddRectFilled({ p.x + ImGui::GetWindowWidth() - 1, p.y }, p + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_Border));

                    ImGui::SetCursorPosY(87);

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 16 });
                    if (custom::tab("A", "Page 1", tab == 0)) tab = 0;
                    if (custom::tab("B", "Page 2", tab == 1)) tab = 1;
                    if (custom::tab("C", "Page 3", tab == 2)) tab = 2;
                    if (custom::tab("D", "Page 4", tab == 3)) tab = 3;
                    if (custom::tab("E", "Page 5", tab == 4)) tab = 4;
                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();

                ImGui::SetCursorPos({ 41.5f + 100 * navbar_width, 47 });
                if (custom::collapse_button(navbar_collapsed)) navbar_collapsed = !navbar_collapsed;
            }
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


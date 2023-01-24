#pragma once

#define BLACKLISTED_PATH "Game.RobloxReplicatedStorage.IntegrityCheckProcessorKey_LocalizationTableEntryStatisticsSender_LocalizationService"

namespace rbx
{
	namespace triggers
	{
		using FireServer = void(__thiscall*)(std::uintptr_t pThis, std::uintptr_t args, std::uint8_t unk);
        
        class Spy
        {
        public:
            FireServer OriginalFireServer = nullptr;

            Spy();

            void Initialize();

            bool Render();
        };

        Spy* SpyManager = nullptr;
        utils::hooks::TrampolineHook* TrampHook = nullptr;

        void __fastcall FireServerHook(std::uintptr_t this_ptr, std::uintptr_t edx, std::uintptr_t args, std::uint8_t unk)
        {
            const auto remote_path = rbx::funcs::GetInstancePath(this_ptr);

            if (remote_path == BLACKLISTED_PATH)
                return;

            std::printf("---START---\n\n");

            std::printf("FireServer Called: %s\n", remote_path.c_str());

            if (!args)
            {
                std::printf("Number of Args: 0\n");
                std::printf("\n---END---\n\n");

                return SpyManager->OriginalFireServer(this_ptr, args, unk);
            }

            HandleVector(args);

            std::printf("\n---END---\n\n");

            return SpyManager->OriginalFireServer(this_ptr, args, unk);
        }

        Spy::Spy() { SpyManager = this; }

        void Spy::Initialize() {

            // pattern scan FireServer & InvokeServer
            __Warn("FindFireServer");
            rbx::offsets::FindFireServer();
            __Ok("Pattern scan ok");

            // place detour
            TrampHook = new utils::hooks::TrampolineHook(
                rbx::offsets::fire_server,
                (uintptr_t)rbx::triggers::FireServerHook,
                6
            );

            //const auto hookAddr = TrampHook->Place();

            //this->OriginalFireServer = reinterpret_cast<rbx::triggers::FireServer>(hookAddr);
        }

        bool Spy::Render() {

            return true;
        }
	}
}
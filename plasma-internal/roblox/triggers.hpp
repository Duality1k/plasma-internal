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
            std::printf("FireServerDetour\n");
            const auto instance = reinterpret_cast<rbx::sdk::Instance*>(this_ptr);
            std::printf("Instance: 0x%x\n", instance);
            const auto remote_path = rbx::funcs::GetInstancePath(instance);
            std::printf("FireServer Called: %s\n", remote_path.c_str());

            if (remote_path == BLACKLISTED_PATH)
                return;

            if (!args)
            {
                std::printf("Number of Args: 0\n");

                return SpyManager->OriginalFireServer(this_ptr, args, unk);
            }

            //HandleVector(args);
            std::printf("args: 0x%x", args);
            const auto args1 = memory::read<uintptr_t>(args);
            std::printf("args1: 0x%x", args1);

            return SpyManager->OriginalFireServer(this_ptr, args, unk);
        }

        Spy::Spy() { SpyManager = this; }

        void Spy::Initialize() {

            // place detour
            TrampHook = new utils::hooks::TrampolineHook(
                rbx::addresses::fire_server,
                (uintptr_t)rbx::triggers::FireServerHook,
                6
            );

            const auto hookAddr = TrampHook->Place();

            this->OriginalFireServer = reinterpret_cast<rbx::triggers::FireServer>(hookAddr);
        }

        bool Spy::Render() {

            return true;
        }
	}
}
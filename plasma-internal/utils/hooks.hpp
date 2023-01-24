#pragma once

namespace utils
{
	namespace hooks
	{
        class TrampolineHook
        {
            bool should_revert = false;

            std::uintptr_t func = 0x0;
            std::uintptr_t new_func = 0x0;
            std::size_t inst_size = 0x0;

            std::uintptr_t clone = 0x0;

            std::uint8_t patch_1 = 0x0;
            std::uintptr_t patch_2 = 0x0;
            void* patch_3 = 0x0;
            std::uint8_t patch_4 = 0x0;
            std::uintptr_t patch_5 = 0x0;

        public:
            TrampolineHook(std::uintptr_t func, std::uintptr_t new_func, std::size_t inst_size);
            std::uintptr_t Place();
            void AwaitRevertTransaction();
            void DetourInitRevertTransaction();
        };

        TrampolineHook::TrampolineHook(std::uintptr_t func, std::uintptr_t new_func, std::size_t inst_size)
        {
            this->func = func;
            this->new_func = new_func;
            this->inst_size = inst_size;
        }

        std::uintptr_t TrampolineHook::Place()
        {
            constexpr auto extra_size = 5;

            clone = reinterpret_cast<std::uintptr_t>(VirtualAlloc(nullptr, inst_size + extra_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

            if (!clone)
                return 0;

            std::memmove(reinterpret_cast<void*>(clone), reinterpret_cast<void*>(func), inst_size);
            
            //this->patch_1 = memory::read<std::uint8_t>(clone + inst_size); // recovery patch 1
            memory::write<std::uint8_t>(clone + inst_size, 0xE9);
            //this->patch_2 = memory::read<std::uint8_t>(clone + inst_size + 1); // recovery patch 2
            memory::write<std::uintptr_t>(clone + inst_size + 1, func - clone - extra_size);

            DWORD old_protect{ 0 };

            VirtualProtect(reinterpret_cast<void*>(func), inst_size, PAGE_EXECUTE_READWRITE, &old_protect);

            //std::memcpy(this->patch_3, reinterpret_cast<void*>(func), inst_size); // recovery patch 3
            std::memset(reinterpret_cast<void*>(func), 0x90, inst_size);

            //this->patch_4 = memory::read<std::uint8_t>(func); // recovery patch 4
            memory::write<std::uint8_t>(func, 0xE9);
            //this->patch_5 = memory::read<std::uint8_t>(func + 1); // recovery patch 5
            memory::write<std::uintptr_t>(func + 1, new_func - func - extra_size);

            VirtualProtect(reinterpret_cast<void*>(func), inst_size, old_protect, &old_protect);

            return clone;
        }

        void TrampolineHook::AwaitRevertTransaction()
        {
            //std::thread{ [&]() {
            //    while (!should_revert)
            //    {
            //        DWORD old_protect{ 0 };
            //        VirtualProtect(reinterpret_cast<void*>(func), inst_size, PAGE_EXECUTE_READWRITE, &old_protect);

            //        memory::write<std::uint8_t>(func, patch_4);
            //        memory::write<std::uintptr_t>(func + 1, patch_5);
            //        std::memcpy(reinterpret_cast<void*>(func), patch_3, inst_size);

            //        memory::write<std::uint8_t>(clone + inst_size, patch_1);
            //        memory::write<std::uintptr_t>(clone + inst_size + 1, patch_2);

            //        VirtualProtect(reinterpret_cast<void*>(func), inst_size, old_protect, &old_protect);

            //        VirtualFree(reinterpret_cast<void*>(new_func), inst_size + 5, MEM_RELEASE);
            //    }
            //} }.detach();
        }

        void TrampolineHook::DetourInitRevertTransaction()
        {
            should_revert = true;
        }
	}
}
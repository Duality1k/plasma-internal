#pragma once

namespace rbx
{
    namespace typedefs
    {
        using luau_load = int(__fastcall*)(std::uintptr_t rL, std::string* source, const char* chunkname, int env);
        using task_defer = int(__cdecl*)(std::uintptr_t rL);
        using get_task_scheduler = int(__cdecl*)();
    }

    namespace addresses
    {
        const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
        const std::uintptr_t luavm_load(base + 0x33C840);
        const std::uintptr_t task_defer(base + 0x3B50B0);
        const std::uintptr_t fire_server(base + 0x1076050);
        const std::uintptr_t task_scheduler(base + 0x6F5CF0);
    }

	namespace offsets
	{
        // instances
		constexpr std::uint16_t parent(0x34);
		constexpr std::uint16_t name(0x28);
		constexpr std::uint16_t string_length(0x14);

        // task scheduler
        constexpr std::uintptr_t job_start(0x134);
        constexpr std::uintptr_t job_end(0x138);
        constexpr std::uintptr_t job_name(0x10);

        constexpr std::uintptr_t script_context(0x130);
	}

    namespace sdk
    {
        typedef UINT* vftable;

        struct ClassDescriptor {
            char padding2[0x4];
            std::string* class_name;
        };

        struct Instance {
            vftable vftable;
            std::shared_ptr<Instance> self;
            ClassDescriptor* classDescriptor;
            char padding1[0x14];
            std::string* name;
            std::vector<Instance*>* children;
            char padding2[0x4];
            Instance* parent;
        };
    }

	namespace funcs
	{
        namespace lua
        {
            const auto luavm_load = reinterpret_cast<typedefs::luau_load>(addresses::luavm_load);
            const auto task_defer = reinterpret_cast<typedefs::task_defer>(addresses::task_defer);
            const auto get_task_scheduler = reinterpret_cast<typedefs::get_task_scheduler>(addresses::task_scheduler);

            std::uintptr_t getstate(std::uintptr_t ScriptContext) {
                // needs to be updated every wednesday
                return 0;
            }
        }

        rbx::sdk::Instance* GetInstanceParent(rbx::sdk::Instance* instance) { return instance->parent; }

        std::string GetInstanceName(rbx::sdk::Instance* instance) { return instance->name->c_str(); }

        std::string GetInstancePath(rbx::sdk::Instance* instance)
        {
            auto path = GetInstanceName(instance);
            auto parent = GetInstanceParent(instance);

            while (parent)
            {
                path = GetInstanceName(parent) + "." + path;
                parent = GetInstanceParent(parent);
            }

            return path;
        }
	}
}

#include "scheduler.hpp"
#include "triggers.hpp"
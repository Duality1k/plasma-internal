#pragma once


namespace rbx
{
	const std::uintptr_t BaseAddress = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
      	const std::uintptr_t LuaVM_load = BaseAddress + 0x33C840;
        const std::uintptr_t defer = BaseAddress + 0x3B50B0;
	const std::uintptr_t fire_server = BaseAddress + 0xF16050;
	
	namespace offsets
	{	
		constexpr std::uint16_t parent = 0x34;
		constexpr std::uint16_t name = 0x28;
		constexpr std::uint16_t string_length = 0x14;

		void FindFireServer() {
		}
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
        std::string ReadUnknownLengthString(std::uintptr_t address)
        {
            std::string res;
            char character = 0;
            int char_size = sizeof(character);
            int offset = 0;

            res.reserve(204); // 4 * 51 = 204

            while (offset < 200)
            {
                character = *reinterpret_cast<char*>(address + offset);

                if (character == 0) break;

                offset += char_size;

                res.push_back(character);
            }

            return res;
        }

        std::string ReadString(std::uintptr_t str_address)
        {
            const auto length = memory::read<int>(str_address + rbx::offsets::string_length);

            if (length >= 16u)
            {
                const auto new_name_address = *reinterpret_cast<std::uintptr_t*>(str_address);
                return ReadUnknownLengthString(new_name_address);
            }
            else
            {
                const auto name = ReadUnknownLengthString(str_address);
                return name;
            }
        }

        rbx::sdk::Instance* GetInstanceParent(rbx::sdk::Instance* instance)
        {
            return instance->parent;
        }

        std::string GetInstanceName(rbx::sdk::Instance* instance)
        {
            return instance->name->c_str();
        }

        std::string GetInstancePath(rbx::sdk::Instance* instance)
        {
            std::printf("GetInstanceName");
            auto path = GetInstanceName(instance);
            std::printf("GetInstanceParent");
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

#include "args_handler.hpp"
#include "triggers.hpp"

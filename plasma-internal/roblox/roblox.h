#pragma once


namespace rbx
{
	inline std::uintptr_t BaseAddress = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));

	namespace offsets
	{
		inline std::uintptr_t invoke_server = NULL;
		inline std::uintptr_t fire_server = NULL;
		
		constexpr std::uint16_t parent = 0x34;
		constexpr std::uint16_t name = 0x28;
		constexpr std::uint16_t string_length = 0x14;

		void FindFireServer() {
            __Warn("invoke_server");
            invoke_server = BaseAddress + 0x1095DB0;
            __Warn("fire_server");
			fire_server = BaseAddress + 0x1098A30;
		}
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

        std::uintptr_t GetInstanceParent(std::uintptr_t address)
        {
            return memory::read<std::uintptr_t>(address + rbx::offsets::parent);
        }

        std::string GetInstanceName(std::uintptr_t address)
        {
            std::uintptr_t name_address = memory::read<std::uintptr_t>(address + rbx::offsets::name);

            return ReadString(name_address);
        }

        std::string GetInstancePath(std::uintptr_t address)
        {
            std::string path = GetInstanceName(address);

            std::uintptr_t parent = GetInstanceParent(address);

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

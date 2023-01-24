#pragma once

namespace utils
{
	namespace memory
	{
		template <typename T>
		static T read(std::uintptr_t address) {
			return *(T*)address;
		}

		template <typename T>
		static void write(std::uintptr_t address, const T& value) {
			*(T*)address = value;
		}

        struct segment
        {
            std::string_view name;
            std::uintptr_t start_addr = 0;
            std::uintptr_t end_addr = 0;
            std::size_t size = 0;

            segment(std::string_view name_s, std::uintptr_t mod = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr))) : name{ name_s }
            {
                __Warn("reading dos");
                const auto pDosHeader = (PIMAGE_DOS_HEADER)mod;
                if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
                    __Warn("Invalid pDos while pattern scanning");
                    return;
                }
                __Ok("Got dos: 0x%x", pDosHeader->e_magic);
                const auto pNtHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)pDosHeader + pDosHeader->e_lfanew);
                if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
                    __Warn("Invalid pNT while pattern scanning: 0x%x", pNtHeaders->Signature);
                    return;
                }
                __Ok("Got nt");

                auto section = reinterpret_cast<IMAGE_SECTION_HEADER*>(pNtHeaders + 1);
                __Ok("section");
                for (auto iteration = 0u; iteration < pNtHeaders->FileHeader.NumberOfSections; ++iteration, ++section)
                {
                    __Ok("segment_name");
                    const auto segment_name = reinterpret_cast<const char*>(section->Name);

                    if (name == segment_name)
                    {
                        start_addr = mod + section->VirtualAddress;
                        size = section->Misc.VirtualSize;
                        end_addr = start_addr + size;
                        __Ok("break...");
                        break;
                    }
                }
            }
        };


        std::vector<std::uintptr_t> PatternScan(const std::string_view& pattern, const std::string_view& mask)
        {
            std::vector<std::uintptr_t> results;

            segment text{ ".text" };

            for (auto at = text.start_addr; at < text.end_addr; ++at)
            {
                const auto is_same = [&]() -> bool
                {
                    for (auto i = 0u; i < mask.length(); ++i)
                    {
                        if (*reinterpret_cast<std::uint8_t*>(at + i) != static_cast<std::uint8_t>(pattern[i]) && mask[i] == 'x')
                        {
                            return false;
                        }
                    }

                    return true;
                };

                if (is_same())
                    results.push_back(at);
            }

            __Ok("results");
            return results;
        }
	}
}
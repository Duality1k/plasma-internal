#pragma once
#include <stdexcept>

// disable console debug
// #define CONSOLE_DEBUG_DISABLED

class Console
{
private:
	// address for the FreeConsole function, initialized in cstor
	std::uintptr_t freeConsoleAddress = NULL;
	// used to 'fix' FreeConsole when calling Detach(), initialized in Attach()
	std::uintptr_t* patchBackup1 = NULL;
	std::uintptr_t patchBackup2 = NULL;

public:
	explicit Console();
	bool SetTitle(std::wstring title);
	void Attach();
	void Detach();
};

Console::Console() {
#ifndef CONSOLE_DEBUG_DISABLED
	const auto k32lib = LoadLibraryW(xor(L"kernel32.dll"));
	if (!k32lib) throw std::runtime_error(xor("LoadLibraryW for kernel32.dll failed"));
	this->freeConsoleAddress = reinterpret_cast<std::uintptr_t>(GetProcAddress(k32lib, xor("FreeConsole")));
	FreeLibrary(k32lib);
#endif
}

bool Console::SetTitle(std::wstring title)
{
#ifndef CONSOLE_DEBUG_DISABLED
	return SetConsoleTitleW(title.c_str());
#endif
}

void Console::Attach()
{
#ifndef CONSOLE_DEBUG_DISABLED
	if (this->freeConsoleAddress)
	{
		DWORD prot{ 0u };
		VirtualProtect(reinterpret_cast<void*>(this->freeConsoleAddress), 0x6, PAGE_EXECUTE_READWRITE, &prot);

		static auto jmp = this->freeConsoleAddress + 0x6;

		this->patchBackup1 = memory::read<std::uintptr_t*>(this->freeConsoleAddress + 0x2);
		this->patchBackup2 = memory::read<std::uintptr_t>(this->freeConsoleAddress + 0x6);

		memory::write<std::uintptr_t*>(this->freeConsoleAddress + 0x2, &jmp);
		memory::write<std::uintptr_t>(this->freeConsoleAddress + 0x6, 0xC3);

		VirtualProtect(reinterpret_cast<void*>(this->freeConsoleAddress), 0x6, prot, &prot);
	}
	else
	{
		throw std::runtime_error(xor("LoadLibraryW address is not initialized or found"));
	}

	AllocConsole();

	FILE* file_stream;
	freopen_s(&file_stream, xor("CONIN$"), xor("r"), stdin);
	freopen_s(&file_stream, xor("CONOUT$"), xor("w"), stdout);
	freopen_s(&file_stream, xor("CONOUT$"), xor("w"), stderr);
#endif
}

void Console::Detach()
{
#ifndef CONSOLE_DEBUG_DISABLED
	if (this->freeConsoleAddress)
	{
		DWORD prot{ 0u };
		VirtualProtect(reinterpret_cast<void*>(this->freeConsoleAddress), 0x6, PAGE_EXECUTE_READWRITE, &prot);

		memory::write<std::uintptr_t*>(this->freeConsoleAddress + 0x2, this->patchBackup1);
		memory::write<std::uintptr_t>(this->freeConsoleAddress + 0x6, this->patchBackup2);

		VirtualProtect(reinterpret_cast<void*>(this->freeConsoleAddress), 0x6, prot, &prot);
	}
	else
	{
		throw std::runtime_error(xor("LoadLibraryW address is not initialized or found"));
	}

	FreeConsole();
#endif
}
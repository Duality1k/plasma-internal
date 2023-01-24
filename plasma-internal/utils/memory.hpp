#pragma once

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
}
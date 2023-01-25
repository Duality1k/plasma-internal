#pragma once

#include <iostream>
#include <array>

#define ARG_STRUCT_SIZE 0x24

namespace rbx
{
    enum class arg_type : std::uint16_t
    {
        t_string,
        t_float,
        t_int,
        t_double,
        t_bool,
        t_none
    };

    struct type_data
    {
        const char* name;
        std::uint32_t arg_amount;
        arg_type type;
        bool special_type;
    };

    auto roblox_types = std::to_array<type_data>
        ({
            {"Vector3", 3, arg_type::t_float, false},
            {"Vector2", 2, arg_type::t_float, false},
            {"CoordinateFrame", 12, arg_type::t_float, false},
            {"string", 1, arg_type::t_string, false},
            {"double", 1, arg_type::t_double, false},
            {"bool", 1, arg_type::t_bool, false},
            {"Color3", 3, arg_type::t_float, false},
            {"Ray", 6, arg_type::t_float, false},
            {"Instance", 1, arg_type::t_none, true},
            {"BrickColor", 1, arg_type::t_int, false}
            });

    struct read_data
    {
        std::uint8_t data[0x30];
    };

    using get_type_t = void(*)(std::uintptr_t type, read_data& out);

    void ReadArg(std::uint32_t index, std::uintptr_t arg, const char* const arg_type_name)
    {
        const auto get_value = reinterpret_cast<get_type_t>(*reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(arg + 4)));

        read_data data{ };

        get_value(arg + 8, data);

        const auto element_p = std::find_if(std::begin(roblox_types), std::end(roblox_types), [&](type_data const& type) { return std::strcmp(type.name, arg_type_name) == 0; });

        if (element_p == std::end(roblox_types))
        {
            std::printf("Unsupported Type\n");

            return;
        }

        std::printf("Arg Value %i: { ", index);

        const auto& element = element_p[0];

        if (element.special_type)
        {
            if (std::strcmp(element.name, "Instance") == 0)
            {
                const auto instance = *reinterpret_cast<rbx::sdk::Instance**>(data.data);

                const auto name = funcs::GetInstancePath(instance);

                std::printf("%s", name.c_str());
            }

            std::printf(" }\n");

            return;
        }

        for (auto i = 0u; i < element.arg_amount; ++i)
        {
            const auto spacing = i == element.arg_amount - 1 ? "" : ", ";

            if (element.type == arg_type::t_float)
            {
                const auto arg = reinterpret_cast<std::uintptr_t>(data.data) + i * sizeof(float);

                std::printf("%f%s", *reinterpret_cast<float*>(arg), spacing);
            }
            else if (element.type == arg_type::t_string)
            {
                const auto arg = reinterpret_cast<std::uintptr_t>(data.data);

                std::printf("%s%s", rbx::funcs::ReadString(arg).c_str(), spacing);
            }
            else if (element.type == arg_type::t_int)
            {
                const auto arg = reinterpret_cast<std::uintptr_t>(data.data) + i * sizeof(std::uint32_t);

                std::printf("%i%s", *reinterpret_cast<std::uint32_t*>(arg), spacing);
            }
            else if (element.type == arg_type::t_bool)
            {
                const auto arg = reinterpret_cast<std::uintptr_t>(data.data) + i * sizeof(std::uint8_t);

                std::printf("%s%s", *reinterpret_cast<std::uint8_t*>(arg) ? "true" : "false", spacing);
            }
            else if (element.type == arg_type::t_double)
            {
                const auto arg = reinterpret_cast<std::uintptr_t>(data.data) + i * sizeof(double);

                std::printf("%f%s", *reinterpret_cast<double*>(arg), spacing);
            }
        }

        std::printf(" }\n");
    }

    void HandleVector(std::uintptr_t args, bool is_deep = false)
    {
        std::printf("HandleVector, args: 0x%x\n", args);
        getchar();
        const auto vector_start = *reinterpret_cast<std::uintptr_t*>(args);
        const auto vector_end = *reinterpret_cast<std::uintptr_t*>(args + 4);

        const auto vector_size = vector_end - vector_start;
        const auto number_of_args = vector_size / ARG_STRUCT_SIZE;

        if (is_deep)
            std::printf("\n{\n");

        std::printf("Number of Args: %i\n", number_of_args);

        for (auto i = 0u; i < number_of_args; ++i)
        {
            const auto arg = vector_start + (i * ARG_STRUCT_SIZE);

            const auto arg_type = *reinterpret_cast<std::uintptr_t*>(arg);
            std::printf("pArgType 0x%x\n", arg);
            const auto arg_type_name = memory::read<std::string>(arg_type + 4);
            std::printf("ArgType %i: %s\n", i, arg_type_name.c_str());


            if (std::strcmp(arg_type_name.c_str(), "Array") == 0)
            {
                const auto vector = *reinterpret_cast<std::uintptr_t*>(arg + 8);

                HandleVector(vector, true);

                continue;
            }

            if (std::strcmp(arg_type_name.c_str(), "void") == 0)
            {
                std::printf("Arg Value %i: { ", i);

                std::printf("None");

                std::printf(" }\n");

                continue;
            }

            if (std::strcmp(*reinterpret_cast<const char**>(arg_type + 0x18), "token") == 0) //Enums
            {
                std::printf("Arg Value %i: { ", i);

                std::printf("%i", *reinterpret_cast<std::uint32_t*>(arg + 8));

                std::printf(" }\n");

                continue;
            }

            ReadArg(i, arg, arg_type_name.c_str());
        }

        if (is_deep)
            std::printf("}\n");
    }
}
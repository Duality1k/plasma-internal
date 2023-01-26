#pragma once

#include "roblox/compression/xxhash.h"
#include "roblox/compression/zstd.h"

#include "luau/Compiler.h"
#include "luau/BytecodeBuilder.h"

namespace rbx
{
    namespace executor
    {
        class opcode_encoder : public Luau::BytecodeEncoder {
            std::uint8_t encodeOp(const std::uint8_t opcode) {
                return opcode * 227;
            }
        };

        // Thank you Nezy!
        std::string compress(const std::string& data) {
            std::string output = "RSB1";
            std::size_t dataSize = data.size();
            std::size_t maxSize = ZSTD_compressBound(dataSize);
            std::vector<char> compressed(maxSize);
            std::size_t compSize = ZSTD_compress(&compressed[0], maxSize, data.c_str(), dataSize, ZSTD_maxCLevel());
            output.append(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
            output.append(&compressed[0], compSize);
            std::uint32_t firstHash = XXH32(&output[0], output.size(), 42U);
            std::uint8_t hashedBytes[4];
            std::memcpy(hashedBytes, &firstHash, sizeof(firstHash));
            for (std::size_t i = 0; i < output.size(); ++i)
                output[i] ^= hashedBytes[i % 4] + i * 41U;
            return output;
        }

        void execute(const std::string source, const std::uintptr_t scriptContext)
        {
            std::uintptr_t rL = rbx::funcs::lua::getstate(NULL); // insert scriptcontext here
            opcode_encoder encoder{};
            std::string compiled = Luau::compile(source, {}, {}, &encoder);
            std::uint8_t version = compiled.c_str()[0];
            if (version < LBC_VERSION_MIN || version > LBC_VERSION_MAX) {
                const char* error_message = compiled.c_str() + 1;
                // syntax error
                return;
            }
            std::string compressed = compress(compiled);
            rbx::funcs::lua::luavm_load(rL, &compressed, xor("=Plasma"), NULL);
            rbx::funcs::lua::task_defer(rL);
        }
    }
}
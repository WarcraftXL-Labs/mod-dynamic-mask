/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DYNAMIC_BIT_MASK_H
#define DYNAMIC_BIT_MASK_H

#if __has_include("Define.h")
#include "Define.h"
#else
#include <cstdint>
typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;
#endif
#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Dynamic arbitrary-length bitmask with Small Buffer Optimization (SBO).
 *
 * Stores words in Little-Endian order in memory (_words[0] holds bits 0..31, etc.)
 * and serializes/deserializes to/from Big-Endian binary byte arrays (matching MySQL VARBINARY & and |).
 *
 * For playable race IDs:
 * Race 1 (Human)    -> bit 0 (1 << 0)
 * Race 2 (Orc)      -> bit 1 (1 << 1)
 * Race 32           -> bit 31 (1 << 31 in _words[0])
 * Race 33           -> bit 32 (1 << 0 in _words[1])
 */
class DynamicBitMask
{
public:
    DynamicBitMask() = default;

    explicit DynamicBitMask(uint32 legacyMask)
    {
        if (legacyMask != 0)
        {
            _words.push_back(legacyMask);
        }
    }

    DynamicBitMask(uint8 const* data, std::size_t size)
    {
        FromBinary(data, size);
    }

    explicit DynamicBitMask(std::vector<uint8> const& bytes)
    {
        FromBinary(bytes);
    }

    void Clear()
    {
        _words.clear();
    }

    [[nodiscard]] bool IsEmpty() const
    {
        for (uint32 word : _words)
        {
            if (word != 0)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool HasBit(std::size_t bit) const
    {
        std::size_t const w = bit / 32;
        std::size_t const b = bit % 32;
        if (w >= _words.size())
        {
            return false;
        }
        return (_words[w] & (1U << b)) != 0;
    }

    void SetBit(std::size_t bit, bool set = true)
    {
        std::size_t const w = bit / 32;
        std::size_t const b = bit % 32;
        if (w >= _words.size())
        {
            if (!set)
            {
                return;
            }
            _words.resize(w + 1, 0);
        }

        if (set)
        {
            _words[w] |= (1U << b);
        }
        else
        {
            _words[w] &= ~(1U << b);
        }
    }

    [[nodiscard]] bool HasRace(uint32 raceId) const
    {
        if (raceId == 0)
        {
            return false;
        }
        return HasBit(raceId - 1);
    }

    void SetRace(uint32 raceId, bool set = true)
    {
        if (raceId == 0)
        {
            return;
        }
        SetBit(raceId - 1, set);
    }

    [[nodiscard]] bool MatchesRace(uint32 raceId) const
    {
        return HasRace(raceId);
    }

    [[nodiscard]] bool HasClass(uint32 classId) const
    {
        if (classId == 0)
        {
            return false;
        }
        return HasBit(classId - 1);
    }

    void SetClass(uint32 classId, bool set = true)
    {
        if (classId == 0)
        {
            return;
        }
        SetBit(classId - 1, set);
    }

    [[nodiscard]] bool MatchesClass(uint32 classId) const
    {
        return HasClass(classId);
    }

    [[nodiscard]] bool MatchesLegacyMask(uint32 raceMask) const
    {
        if (_words.empty())
        {
            return raceMask == 0;
        }
        return (_words[0] & raceMask) != 0;
    }

    [[nodiscard]] bool Matches(DynamicBitMask const& other) const
    {
        std::size_t const count = std::min(_words.size(), other._words.size());
        for (std::size_t i = 0; i < count; ++i)
        {
            if ((_words[i] & other._words[i]) != 0)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32 GetLegacyMask() const
    {
        return _words.empty() ? 0 : _words[0];
    }

    [[nodiscard]] std::size_t GetWordCount() const
    {
        return _words.size();
    }

    [[nodiscard]] uint32 GetWord(std::size_t index) const
    {
        return index < _words.size() ? _words[index] : 0;
    }

    void FromBinary(uint8 const* data, std::size_t size)
    {
        _words.clear();
        if (!data || size == 0)
        {
            return;
        }

        std::size_t const totalBytes = size;
        std::size_t const totalWords = (totalBytes + 3) / 4;
        _words.resize(totalWords, 0);

        for (std::size_t i = 0; i < totalBytes; ++i)
        {
            std::size_t const revIdx = totalBytes - 1 - i;
            std::size_t const w = revIdx / 4;
            std::size_t const bInW = revIdx % 4;
            _words[w] |= static_cast<uint32>(data[i]) << (bInW * 8);
        }

        while (_words.size() > 1 && _words.back() == 0)
        {
            _words.pop_back();
        }
    }

    void FromBinary(std::vector<uint8> const& bytes)
    {
        FromBinary(bytes.data(), bytes.size());
    }

    [[nodiscard]] std::vector<uint8> ToBinary() const
    {
        std::vector<uint8> bytes;
        if (_words.empty())
        {
            return bytes;
        }

        std::size_t lastWord = _words.size();
        while (lastWord > 0 && _words[lastWord - 1] == 0)
        {
            --lastWord;
        }

        if (lastWord == 0)
        {
            return { 0 };
        }

        uint32 const top = _words[lastWord - 1];
        std::size_t topBytes = 4;
        if ((top & 0xFF000000U) == 0)
        {
            topBytes = 3;
            if ((top & 0x00FF0000U) == 0)
            {
                topBytes = 2;
                if ((top & 0x0000FF00U) == 0)
                {
                    topBytes = 1;
                }
            }
        }

        std::size_t const totalBytes = (lastWord - 1) * 4 + topBytes;
        bytes.resize(totalBytes, 0);

        for (std::size_t i = 0; i < totalBytes; ++i)
        {
            std::size_t const revIdx = totalBytes - 1 - i;
            std::size_t const w = revIdx / 4;
            std::size_t const bInW = revIdx % 4;
            bytes[i] = static_cast<uint8>((_words[w] >> (bInW * 8)) & 0xFF);
        }

        return bytes;
    }

    [[nodiscard]] std::string ToHexString() const
    {
        std::vector<uint8> bytes = ToBinary();
        if (bytes.empty())
        {
            return "0x00";
        }

        std::ostringstream ss;
        ss << "0x";
        for (uint8 b : bytes)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        return ss.str();
    }

private:
    boost::container::small_vector<uint32, 2> _words;
};

#endif // DYNAMIC_BIT_MASK_H

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

#ifndef DYNAMIC_MASK_MGR_H
#define DYNAMIC_MASK_MGR_H

#include "Define.h"
#include "DynamicBitMask.h"
#include <string>
#include <unordered_map>

inline uint64 MakeDynamicMaskOverlayKey(uint8 table, uint32 recordId, uint8 maskIndex)
{
    return (uint64(table) << 40) | (uint64(maskIndex) << 32) | uint64(recordId);
}

class DynamicMaskMgr
{
public:
    static DynamicMaskMgr* instance();

    void LoadFromDB();
    void LoadWDBC(std::string const& dataPath);

    /**
     * @brief Checks if a DBC record has a dynamic racemask overlay and evaluates it against race.
     */
    bool CheckDBCRace(uint8 table, uint32 recordId, uint8 maskIndex, uint8 race, bool& allowed) const;

    /**
     * @brief Checks if a DBC record has a dynamic classmask overlay and evaluates it against class_.
     */
    bool CheckDBCClass(uint8 table, uint32 recordId, uint8 maskIndex, uint8 class_, bool& allowed) const;

    /**
     * @brief Checks if an item has a dynamic racemask overlay and evaluates it against raceId.
     */
    bool CheckItemRace(uint32 entry, uint32 raceId, bool& allowed) const;

    /**
     * @brief Checks if a quest has a dynamic racemask overlay and evaluates it against raceId.
     */
    bool CheckQuestRace(uint32 entry, uint32 raceId, bool& allowed) const;

    /**
     * @brief Checks if a condition has a dynamic racemask overlay and evaluates it against raceId.
     */
    bool CheckConditionRace(uint32 condId, uint32 raceId, bool& meets) const;

    /**
     * @brief Validates that a condition overlay ID exists in dynamic_racemask_condition.
     */
    [[nodiscard]] bool ValidateConditionRace(uint32 condId) const;

    [[nodiscard]] DynamicBitMask const* GetItemMask(uint32 entry) const;
    [[nodiscard]] DynamicBitMask const* GetQuestMask(uint32 entry) const;
    [[nodiscard]] DynamicBitMask const* GetConditionMask(uint32 condId) const;
    [[nodiscard]] DynamicBitMask const* GetDynamicOverlayMask(uint8 table, uint32 recordId, uint8 maskIndex = 0) const;

    [[nodiscard]] std::size_t GetItemMaskCount() const { return _itemMasks.size(); }
    [[nodiscard]] std::size_t GetQuestMaskCount() const { return _questMasks.size(); }
    [[nodiscard]] std::size_t GetConditionMaskCount() const { return _conditionMasks.size(); }
    [[nodiscard]] std::size_t GetOverlayMaskCount() const { return _overlayMasks.size(); }

private:
    DynamicMaskMgr() = default;
    ~DynamicMaskMgr() = default;

    std::unordered_map<uint32, DynamicBitMask> _itemMasks;
    std::unordered_map<uint32, DynamicBitMask> _questMasks;
    std::unordered_map<uint32, DynamicBitMask> _conditionMasks;
    std::unordered_map<uint64, DynamicBitMask> _overlayMasks;
};

#define sDynamicMaskMgr DynamicMaskMgr::instance()

#endif // DYNAMIC_MASK_MGR_H

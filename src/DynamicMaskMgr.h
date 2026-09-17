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
#include <unordered_map>

class DynamicMaskMgr
{
public:
    static DynamicMaskMgr* instance();

    void LoadFromDB();

    /**
     * @brief Checks if an item has a dynamic racemask overlay and evaluates it against raceId.
     * @param entry Item template entry (PK)
     * @param raceId Player race ID (1-based)
     * @param allowed Output whether the race is permitted
     * @return true if an entry exists in dynamic_racemask_item (handled), false otherwise (fallback)
     */
    bool CheckItemRace(uint32 entry, uint32 raceId, bool& allowed) const;

    /**
     * @brief Checks if a quest has a dynamic racemask overlay and evaluates it against raceId.
     * @param entry Quest entry (PK)
     * @param raceId Player race ID (1-based)
     * @param allowed Output whether the race is permitted
     * @return true if an entry exists in dynamic_racemask_quest (handled), false otherwise (fallback)
     */
    bool CheckQuestRace(uint32 entry, uint32 raceId, bool& allowed) const;

    /**
     * @brief Checks if a condition has a dynamic racemask overlay and evaluates it against raceId.
     * @param condId Condition overlay ID (PK)
     * @param raceId Unit race ID (1-based)
     * @param meets Output whether the condition is met
     * @return true if an entry exists in dynamic_racemask_condition (handled), false otherwise (fallback)
     */
    bool CheckConditionRace(uint32 condId, uint32 raceId, bool& meets) const;

    /**
     * @brief Validates that a condition overlay ID exists in dynamic_racemask_condition.
     * @param condId Condition overlay ID (PK)
     * @return true if the ID exists in dynamic_racemask_condition, false otherwise
     */
    [[nodiscard]] bool ValidateConditionRace(uint32 condId) const;

    /**
     * @brief Gets the dynamic racemask overlay for an item entry if one exists.
     * @param entry Item template entry (PK)
     * @return Pointer to DynamicBitMask if found, nullptr otherwise (fallback to legacy)
     */
    [[nodiscard]] DynamicBitMask const* GetItemMask(uint32 entry) const;

    /**
     * @brief Gets the dynamic racemask overlay for a quest entry if one exists.
     * @param entry Quest entry (PK)
     * @return Pointer to DynamicBitMask if found, nullptr otherwise (fallback to legacy)
     */
    [[nodiscard]] DynamicBitMask const* GetQuestMask(uint32 entry) const;

    /**
     * @brief Gets the dynamic racemask overlay for a condition if one exists.
     * @param condId Condition overlay ID (PK)
     * @return Pointer to DynamicBitMask if found, nullptr otherwise (fallback to legacy)
     */
    [[nodiscard]] DynamicBitMask const* GetConditionMask(uint32 condId) const;

    [[nodiscard]] std::size_t GetItemMaskCount() const { return _itemMasks.size(); }
    [[nodiscard]] std::size_t GetQuestMaskCount() const { return _questMasks.size(); }
    [[nodiscard]] std::size_t GetConditionMaskCount() const { return _conditionMasks.size(); }

private:
    DynamicMaskMgr() = default;
    ~DynamicMaskMgr() = default;

    std::unordered_map<uint32, DynamicBitMask> _itemMasks;
    std::unordered_map<uint32, DynamicBitMask> _questMasks;
    std::unordered_map<uint32, DynamicBitMask> _conditionMasks;
};

#define sDynamicMaskMgr DynamicMaskMgr::instance()

#endif // DYNAMIC_MASK_MGR_H

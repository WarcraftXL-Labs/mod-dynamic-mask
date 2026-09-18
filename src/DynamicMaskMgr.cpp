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

#include "DynamicMaskMgr.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "QueryResult.h"
#include "Timer.h"
#include "WdbcFile.h"

DynamicMaskMgr* DynamicMaskMgr::instance()
{
    static DynamicMaskMgr instance;
    return &instance;
}

void DynamicMaskMgr::LoadWDBC(std::string const& dataPath)
{
    uint32 oldMSTime = getMSTime();
    _overlayMasks.clear();

    std::string wdbcPath = dataPath + "dbc/DynamicMasks.wdbc";
    WdbcFile file;
    if (!file.Load(wdbcPath))
    {
        LOG_INFO("server.loading", "DynamicMaskMgr: DynamicMasks.wdbc not found at '{}', using vanilla DBC masks.", wdbcPath);
        return;
    }

    uint32 const count = file.GetRecordCount();
    _overlayMasks.reserve(count);

    for (uint32 r = 0; r < count; ++r)
    {
        uint8 const table = file.GetUInt8(r, 0);
        uint32 const recordId = file.GetUInt32(r, 1);
        uint8 const maskIndex = file.GetUInt8(r, 2);
        std::pair<uint8 const*, uint16> const blob = file.GetBlob(r, 3);
        DynamicBitMask mask(blob.first, blob.second);

        uint64 const key = MakeDynamicMaskOverlayKey(table, recordId, maskIndex);
        _overlayMasks[key] = std::move(mask);
    }

    LOG_INFO("server.loading", ">> Loaded {} dynamic mask overlays from DynamicMasks.wdbc in {} ms",
        _overlayMasks.size(), GetMSTimeDiffToNow(oldMSTime));
}

void DynamicMaskMgr::LoadFromDB()
{
    uint32 oldMSTime = getMSTime();

    _itemMasks.clear();
    _questMasks.clear();
    _conditionMasks.clear();

    // Load item racemasks from HotfixesDatabase
    QueryResult result = HotfixesDatabase.Query("SELECT entry, racemask FROM dynamic_racemask_item");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            std::vector<uint8> racemaskBytes = fields[1].Get<Binary>();

            _itemMasks.emplace(entry, DynamicBitMask(racemaskBytes));
        } while (result->NextRow());
    }

    // Load quest racemasks from HotfixesDatabase
    result = HotfixesDatabase.Query("SELECT entry, racemask FROM dynamic_racemask_quest");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            std::vector<uint8> racemaskBytes = fields[1].Get<Binary>();

            _questMasks.emplace(entry, DynamicBitMask(racemaskBytes));
        } while (result->NextRow());
    }

    // Load condition racemasks from HotfixesDatabase
    result = HotfixesDatabase.Query("SELECT id, racemask FROM dynamic_racemask_condition");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].Get<uint32>();
            std::vector<uint8> racemaskBytes = fields[1].Get<Binary>();

            _conditionMasks.emplace(id, DynamicBitMask(racemaskBytes));
        } while (result->NextRow());
    }

    LOG_INFO("server.loading", ">> Loaded {} dynamic item racemasks, {} quest racemasks, {} condition racemasks in {} ms",
        _itemMasks.size(), _questMasks.size(), _conditionMasks.size(), GetMSTimeDiffToNow(oldMSTime));
}

bool DynamicMaskMgr::CheckDBCRace(uint8 table, uint32 recordId, uint8 maskIndex, uint8 race, bool& allowed) const
{
    uint64 const key = MakeDynamicMaskOverlayKey(table, recordId, maskIndex);
    auto it = _overlayMasks.find(key);
    if (it != _overlayMasks.end())
    {
        allowed = it->second.HasRace(race);
        return true;
    }
    return false;
}

bool DynamicMaskMgr::CheckDBCClass(uint8 table, uint32 recordId, uint8 maskIndex, uint8 class_, bool& allowed) const
{
    uint64 const key = MakeDynamicMaskOverlayKey(table, recordId, maskIndex);
    auto it = _overlayMasks.find(key);
    if (it != _overlayMasks.end())
    {
        allowed = it->second.HasClass(class_);
        return true;
    }
    return false;
}

bool DynamicMaskMgr::CheckItemRace(uint32 entry, uint32 raceId, bool& allowed) const
{
    auto itr = _itemMasks.find(entry);
    if (itr == _itemMasks.end())
    {
        return false;
    }

    allowed = itr->second.HasRace(raceId);
    return true;
}

bool DynamicMaskMgr::CheckQuestRace(uint32 entry, uint32 raceId, bool& allowed) const
{
    auto itr = _questMasks.find(entry);
    if (itr == _questMasks.end())
    {
        return false;
    }

    allowed = itr->second.HasRace(raceId);
    return true;
}

bool DynamicMaskMgr::CheckConditionRace(uint32 condId, uint32 raceId, bool& meets) const
{
    auto itr = _conditionMasks.find(condId);
    if (itr == _conditionMasks.end())
    {
        return false;
    }

    meets = itr->second.HasRace(raceId);
    return true;
}

bool DynamicMaskMgr::ValidateConditionRace(uint32 condId) const
{
    return _conditionMasks.find(condId) != _conditionMasks.end();
}

DynamicBitMask const* DynamicMaskMgr::GetItemMask(uint32 entry) const
{
    auto itr = _itemMasks.find(entry);
    if (itr != _itemMasks.end())
    {
        return &itr->second;
    }
    return nullptr;
}

DynamicBitMask const* DynamicMaskMgr::GetQuestMask(uint32 entry) const
{
    auto itr = _questMasks.find(entry);
    if (itr != _questMasks.end())
    {
        return &itr->second;
    }
    return nullptr;
}

DynamicBitMask const* DynamicMaskMgr::GetConditionMask(uint32 condId) const
{
    auto itr = _conditionMasks.find(condId);
    if (itr != _conditionMasks.end())
    {
        return &itr->second;
    }
    return nullptr;
}

DynamicBitMask const* DynamicMaskMgr::GetDynamicOverlayMask(uint8 table, uint32 recordId, uint8 maskIndex) const
{
    uint64 const key = MakeDynamicMaskOverlayKey(table, recordId, maskIndex);
    auto it = _overlayMasks.find(key);
    if (it != _overlayMasks.end())
    {
        return &it->second;
    }
    return nullptr;
}

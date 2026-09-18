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
#include "Chat.h"
#include "CommandScript.h"
#include "ConditionMgr.h"
#include "Config.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "MiscScript.h"
#include "Player.h"
#include "PlayerScript.h"
#include "QuestDef.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "Unit.h"
#include "WorldPacket.h"
#include "WorldScript.h"
#include "WorldSession.h"

using namespace Acore::ChatCommands;

class DynamicMaskWorldScript : public WorldScript
{
public:
    DynamicMaskWorldScript()
        : WorldScript("DynamicMaskWorldScript", { WORLDHOOK_ON_LOAD_CUSTOM_DATABASE_TABLE })
    {
    }

    void OnLoadCustomDatabaseTable() override
    {

        std::string dataPath = sConfigMgr->GetOption<std::string>("DataDir", "./");
        if (dataPath.empty() || (dataPath.at(dataPath.length() - 1) != '/' && dataPath.at(dataPath.length() - 1) != '\\'))
            dataPath.push_back('/');

        sDynamicMaskMgr->LoadWDBC(dataPath);
        sDynamicMaskMgr->LoadFromDB();
    }
};

class DynamicMaskPlayerScript : public PlayerScript
{
public:
    DynamicMaskPlayerScript()
        : PlayerScript("DynamicMaskPlayerScript", { PLAYERHOOK_ON_CHECK_ITEM_RACE, PLAYERHOOK_ON_CHECK_QUEST_RACE })
    {
    }

    bool OnPlayerCheckItemRace(Player const* player, ItemTemplate const* proto, bool& result) override
    {
        return sDynamicMaskMgr->CheckItemRace(proto->ItemId, player->getRace(true), result);
    }

    bool OnPlayerCheckQuestRace(Player const* player, Quest const* quest, bool& result) override
    {
        return sDynamicMaskMgr->CheckQuestRace(quest->GetQuestId(), player->getRace(true), result);
    }
};

class DynamicMaskMiscScript : public MiscScript
{
public:
    DynamicMaskMiscScript()
        : MiscScript("DynamicMaskMiscScript",
            {
                MISCHOOK_ON_CONDITION_CHECK_RACE,
                MISCHOOK_ON_CONDITION_VALIDATE_RACE,
                MISCHOOK_ON_ITEM_QUERY_SINGLE_RACE_MASK,
                MISCHOOK_ON_CHECK_RACE_MASK,
                MISCHOOK_ON_CHECK_CLASS_MASK
            })
    {
    }

    bool OnCheckRaceMask(uint8 table, uint32 recordId, uint8 maskIndex, uint32 /*fallbackMask*/, uint8 race, bool& result) override
    {
        return sDynamicMaskMgr->CheckDBCRace(table, recordId, maskIndex, race, result);
    }

    bool OnCheckClassMask(uint8 table, uint32 recordId, uint8 maskIndex, uint32 /*fallbackMask*/, uint8 class_, bool& result) override
    {
        return sDynamicMaskMgr->CheckDBCClass(table, recordId, maskIndex, class_, result);
    }

    bool OnConditionCheckRace(Condition const* cond, Unit const* unit, bool& result) override
    {
        // ConditionValue2 == 1 indicates ConditionValue1 is the PK in dynamic_racemask_condition
        if (cond->ConditionValue2 != 1)
        {
            return false; // Fallback to legacy bitmask check
        }

        return sDynamicMaskMgr->CheckConditionRace(cond->ConditionValue1, unit->getRace(true), result);
    }

    bool OnConditionValidateRace(Condition const* cond, bool& result) override
    {
        // ConditionValue2 == 1 indicates ConditionValue1 is the PK in dynamic_racemask_condition
        if (cond->ConditionValue2 != 1)
        {
            return false; // Fallback to legacy validation
        }

        if (cond->ConditionValue3 != 0)
        {
            LOG_ERROR("sql.sql", "Race condition {} has useless data in value3 ({})!", cond->ConditionValue1, cond->ConditionValue3);
        }

        result = sDynamicMaskMgr->ValidateConditionRace(cond->ConditionValue1);
        if (!result)
        {
            LOG_ERROR("sql.sql", "Race condition references non-existing dynamic_racemask_condition id ({}), skipped", cond->ConditionValue1);
        }

        return true; // Handled
    }

    bool OnItemQuerySingleRaceMask(WorldSession* /*session*/, ItemTemplate const* proto, WorldPacket& data) override
    {
        DynamicBitMask const* mask = sDynamicMaskMgr->GetItemMask(proto->ItemId);
        if (mask)
        {
            if (mask->IsEmpty())
            {
                data << uint8(0);
            }
            else
            {
                uint8 const wordCount = static_cast<uint8>(mask->GetWordCount());
                data << wordCount;
                for (std::size_t i = 0; i < wordCount; ++i)
                {
                    data << mask->GetWord(i);
                }
            }
        }
        else
        {
            // Fallback: item has no overlay in dynamic_racemask_item.
            // Serialize legacy proto->AllowableRace in BitPack format so patched client gets consistent data.
            if (proto->AllowableRace == 0)
            {
                data << uint8(0);
            }
            else
            {
                data << uint8(1);
                data << proto->AllowableRace;
            }
        }

        return true;
    }
};

class DynamicMaskCommandScript : public CommandScript
{
public:
    DynamicMaskCommandScript()
        : CommandScript("DynamicMaskCommandScript")
    {
    }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable reloadCommandTable =
        {
            { "dynamic_mask", HandleReloadDynamicMaskCommand, rbac::RBAC_PERM_COMMAND_RELOAD, Console::Yes }
        };

        static ChatCommandTable commandTable =
        {
            { "reload", reloadCommandTable }
        };

        return commandTable;
    }

    static bool HandleReloadDynamicMaskCommand(ChatHandler* handler)
    {
        handler->SendGlobalGMSysMessage("Reloading dynamic bitmask definitions...");
        std::string dataPath = sConfigMgr->GetOption<std::string>("DataDir", "data/");
        if (!dataPath.empty() && dataPath.back() != '/' && dataPath.back() != '\\')
        {
            dataPath += '/';
        }

        sDynamicMaskMgr->LoadWDBC(dataPath);
        sDynamicMaskMgr->LoadFromDB();
        handler->SendGlobalGMSysMessage("Dynamic bitmask definitions reloaded.");
        return true;
    }
};

void Addmod_dynamic_maskScripts()
{
    new DynamicMaskWorldScript();
    new DynamicMaskPlayerScript();
    new DynamicMaskMiscScript();
    new DynamicMaskCommandScript();
}

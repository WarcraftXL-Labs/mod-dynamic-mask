# AzerothCore Module: mod-dynamic-mask

## Overview

`mod-dynamic-mask` is an architectural extension for AzerothCore (WoW 3.3.5a / 12340) designed to break the legacy 32-bit race limit (`RaceID > 32`).

In standard World of Warcraft 3.3.5a, race masks are stored and evaluated as 32-bit unsigned integers (`uint32`), where `bit = 1 << (RaceID - 1)`. Consequently, vanilla and core systems are capped at 32 races (`RaceID` 1 to 32). This module, paired with core ScriptMgr hooks and the dedicated `acore_hotfixes` database, allows servers to define arbitrary-length race bitmasks supporting hundreds of custom races without modifying legacy base tables (`item_template`, `quest_template`, etc.).

---

## Key Features

1. **Arbitrary-Length Race Bitmasks**: Supports custom race IDs well beyond 32 (Race 33, 35, 70, etc.).
2. **Dedicated Hotfixes Database (`acore_hotfixes`)**: Hotfix and dynamic mask data are isolated in a dedicated database worker pool, hot-reloadable at runtime without restarting the worldserver.
3. **Non-Invasive Overlay Pattern**: Legacy content tables (`world.item_template`, `world.quest_template`, `world.conditions`) remain 100% vanilla and intact. Custom masks overlay existing entries by primary key.
4. **Two-Tier Fallback Safety**:
   - **Emulator/Core Fallback**: If the module is not loaded or disabled, AzerothCore executes vanilla 32-bit bitmask logic without any behavioral changes.
   - **Overlay Table Fallback**: If an item, quest, or condition does not have an entry in `dynamic_racemask_*`, the server automatically falls back to legacy 32-bit mask values (`AllowableRace`, `AllowableRaces`, `ConditionValue1`).
5. **Dynamic BitPack Network Streaming**: Efficient serialization for `SMSG_ITEM_QUERY_SINGLE_RESPONSE` to send variable-length race masks to modded WarcraftXL clients.
6. **Zero Allocation for Common Cases (SBO)**: Uses `boost::container::small_vector<uint32, 2>` ensuring stack allocation for up to 64 races (2 words).

---

## Architecture & Database Design

### Hotfixes Database (`acore_hotfixes`)

The module stores its overlay tables in the `acore_hotfixes` database:

```sql
CREATE DATABASE IF NOT EXISTS `acore_hotfixes` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

Connection configuration is managed in `worldserver.conf`:
```ini
HotfixesDatabaseInfo = "127.0.0.1;3306;acore;acore;acore_hotfixes"
HotfixesDatabase.WorkerThreads     = 1
HotfixesDatabase.SynchThreads      = 1
```

### Overlay Tables Schema

All dynamic race masks are stored as `VARBINARY` (byte arrays):

#### 1. Items (`dynamic_racemask_item`)
Overlays race restrictions checked during equipping/using items (`Player::CanUseItem`):
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_item` (
    `entry` INT UNSIGNED NOT NULL COMMENT 'Item template entry (item_template.entry)',
    `racemask` VARBINARY(64) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

#### 2. Quests (`dynamic_racemask_quest`)
Overlays race restrictions checked when viewing or accepting quests (`Player::SatisfyQuestRace`):
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_quest` (
    `entry` INT UNSIGNED NOT NULL COMMENT 'Quest ID (quest_template.ID)',
    `racemask` VARBINARY(64) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

#### 3. Conditions (`dynamic_racemask_condition`)
Overlays `CONDITION_RACE` checks in `ConditionMgr`. Triggered when `ConditionValue2 == 1`, where `ConditionValue1` references `dynamic_racemask_condition.id`:
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_condition` (
    `id` INT UNSIGNED NOT NULL COMMENT 'Condition dynamic racemask ID (referenced by ConditionValue1 when ConditionValue2 = 1)',
    `racemask` VARBINARY(64) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
    `comment` VARCHAR(255) DEFAULT NULL COMMENT 'Description / human readable notes',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

---

## Dynamic Bitmask Rules

The race ID mapping follows the classic Warcraft bit index formula:

$$\text{Bit Index} = \text{RaceID} - 1$$

- **Race 1 (Human)**: Bit 0 (`1 << 0`) $\rightarrow$ Word 0
- **Race 2 (Orc)**: Bit 1 (`1 << 1`) $\rightarrow$ Word 0
- **Race 32**: Bit 31 (`1 << 31`) $\rightarrow$ Word 0
- **Race 33**: Bit 32 (`1 << 0`) $\rightarrow$ Word 1
- **Race 35**: Bit 34 (`1 << 2`) $\rightarrow$ Word 1

### SQL Binary Encoding (Big-Endian)

MySQL handles binary bitwise operators (`&`, `|`) in Big-Endian order. `DynamicBitMask` automatically converts SQL Big-Endian byte arrays to Little-Endian 32-bit words in memory:

- Race 1 only: `0x01`
- Race 35 only (bit 34): `0x0400000000` (5 bytes)
- Race 1 + Race 35: `0x0400000001`

---

## Fallback Mechanisms

To ensure zero regressions and 100% backward compatibility, two fallback levels are enforced:

### 1. Core / Module Level Fallback
All hooks in `ScriptMgr` are non-intrusive and return a boolean indicating whether the hook handled the check:
- `sScriptMgr->OnPlayerCheckItemRace(player, proto, allowed)`
- `sScriptMgr->OnPlayerCheckQuestRace(player, quest, allowed)`
- `sScriptMgr->OnConditionCheckRace(cond, unit, result)`
- `sScriptMgr->OnItemQuerySingleRaceMask(session, proto, queryData)`

If the module is not loaded or disabled, these hooks return `false`, causing AzerothCore to immediately evaluate the default 32-bit expressions:
- `(proto->AllowableRace & player->getRaceMask()) != 0`
- `(quest->GetAllowableRaces() & player->getRaceMask()) != 0`
- `(cond->ConditionValue1 & unit->getRaceMask()) != 0`
- `queryData << proto->AllowableRace;`

### 2. Overlay Entity Fallback
When the module is running:
- If an item `itemId` is not present in `dynamic_racemask_item`, `CheckItemRace` returns `false`, falling back to `proto->AllowableRace`.
- If a quest `questId` is not present in `dynamic_racemask_quest`, `CheckQuestRace` returns `false`, falling back to `quest->GetAllowableRaces()`.
- If a condition row has `ConditionValue2 != 1`, `OnConditionCheckRace` returns `false`, falling back to `cond->ConditionValue1`.
- For item query network serialization (`OnItemQuerySingleRaceMask`), missing items automatically serialize `proto->AllowableRace` packaged in the BitPack format (`wordCount = 1`, `word0 = AllowableRace`).

---

## Configuration

Settings can be customized in `mod_dynamic_mask.conf` (copied from `conf/mod_dynamic_mask.conf.dist`):

```ini
[worldserver]
# DynamicMask.EnableItemOpcodeHook
#   1: Enable custom BitPack serialization in SMSG_ITEM_QUERY_SINGLE_RESPONSE (for patched clients)
#   0: Send vanilla 32-bit AllowableRace integer (for unpatched/vanilla clients)
DynamicMask.EnableItemOpcodeHook = 1
```

---

## Administration & GM Commands

The module registers a reload command requiring GM permissions (`RBAC_PERM_COMMAND_RELOAD`):

```text
.reload dynamic_mask
```

This command re-reads `dynamic_racemask_item`, `dynamic_racemask_quest`, and `dynamic_racemask_condition` from `acore_hotfixes` on the fly without server downtime.

---

## Network Protocol Specification

For full details regarding network serialization, packet layout, and client-side patch integration, see [NETWORK_PROTOCOL.md](NETWORK_PROTOCOL.md).


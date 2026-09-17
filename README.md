# AzerothCore Module: mod-dynamic-mask

## Overview

`mod-dynamic-mask` is an architectural extension for AzerothCore (WoW 3.3.5a / 12340) designed to break the legacy 32-bit race and class limit (`RaceID > 32`, `ClassID > 32`).

In standard World of Warcraft 3.3.5a, race and class masks are stored and evaluated as 32-bit unsigned integers (`uint32`), where `bit = 1 << (ID - 1)`. Consequently, vanilla and core systems are strictly capped at 32 races and classes.

This module provides a unified hybrid architecture combining:
1. **DBC Layer (`DynamicMask.dbc`)**: Replaces fixed 32-bit integer fields across client/server DBCs with dynamic mask IDs.
2. **Hotfixes Database (`acore_hotfixes`)**: Hotfix overlay tables for items, quests, and conditions, hot-reloadable at runtime without restarts.
3. **Core ScriptMgr Hooks**: Non-invasive hooks in item equip, quest prerequisites, conditions, and network packet streaming.
4. **Automated Tooling (`lua-dbc`)**: High-performance Lua scripts in `data/lua/` to parse, generate, and convert DBC files.

---

## Key Features

- **Arbitrary-Length Bitmasks**: Supports custom race IDs well beyond 32 (e.g. Race 33, 35, 70) and custom class IDs.
- **Dedicated Hotfixes Database (`acore_hotfixes`)**: Isolated database worker pool for dynamic mask overlays.
- **Client & Server DBC Integration (`DynamicMask.dbc`)**: Pre-converted DBCs included in `data/dbc/` for seamless client-server parity.
- **Two-Tier Fallback Safety**:
  - **DBC Fallback**: If an integer in a DBC does not match an entry in `DynamicMask.dbc`, `MatchesRaceMask` / `MatchesClassMask` automatically evaluates it as a legacy 32-bit bitmask.
  - **Core Fallback**: If the module is not loaded or disabled, AzerothCore executes vanilla 32-bit bitmask logic without any behavioral changes.
  - **Overlay Table Fallback**: If an item, quest, or condition does not have a row in `dynamic_racemask_*`, the server falls back to legacy 32-bit fields (`AllowableRace`, `AllowableRaces`, `ConditionValue1`).
- **Dynamic BitPack Network Streaming**: Efficient length-prefixed serialization in `SMSG_ITEM_QUERY_SINGLE_RESPONSE` (`0x058`).
- **Zero Allocation for Common Cases (SBO)**: Uses `boost::container::small_vector<uint32, 2>` ensuring stack allocation for up to 64 races (2 words).

---

## DBC Layer: `DynamicMask.dbc`

### DBC Record Structure

The module introduces `DynamicMask.dbc` (`WDBC` format):

| Field Index | Field Name | Type | Description |
|:-----------:|:----------:|:----:|:------------|
| 0 | `ID` | `uint32` | Unique identifier (Primary Key) |
| 1 | `Type` | `uint32` | `0` = RaceMask, `1` = ClassMask, `2` = Other |
| 2 | `Mask` | `string` | Arbitrary-length bitmask string (e.g. `"0x0000044D"`) |
| 3 | `Comment` | `string` | Human-readable description (e.g. `"Alliance"`, `"Horde"`) |

- **RaceMask IDs (`Type = 0`)**: `1` to `999`
- **ClassMask IDs (`Type = 1`)**: `1001` to `1999`

### Modified Stock DBCs Included in `data/dbc/`

The module bundles pre-converted stock 3.3.5a DBC files where legacy 32-bit mask fields have been mapped to `DynamicMask.dbc` IDs:

1. **`SkillRaceClassInfo.dbc`**: Replaced `RaceMask` and `ClassMask`.
2. **`SkillLineAbility.dbc`**: Replaced `RaceMask`, `ExcludeRace`, `ClassMask`, `ExcludeClass`.
3. **`TalentTab.dbc`**: Replaced `RaceMask` and `ClassMask`.
4. **`DanceMoves.dbc`**: Replaced `Racemask`.
5. **`Faction.dbc`**: Replaced `ReputationRaceMask[4]` and `ReputationClassMask[4]`.

### C++ Core Integration & Fallback

In AzerothCore (`DBCStores.h` / `DBCStores.cpp`), masks are validated using global helper functions:

```cpp
bool MatchesRaceMask(uint32 maskOrId, uint8 race);
bool MatchesClassMask(uint32 maskOrId, uint8 class_);
```

**Evaluation Logic**:
1. If `maskOrId == 0`, unrestricted $\rightarrow$ returns `true`.
2. If `maskOrId` is found in `sDynamicMaskCache` (populated from `DynamicMask.dbc`), evaluates `mask->HasRace(race)`.
3. **Fallback**: If `maskOrId` is not found in `DynamicMask.dbc`, evaluates as legacy 32-bit mask: `(maskOrId & (1U << (race - 1))) != 0`.

---

## Database Architecture: `acore_hotfixes`

### Connection Configuration

Managed in `worldserver.conf`:
```ini
HotfixesDatabaseInfo = "127.0.0.1;3306;acore;acore;acore_hotfixes"
HotfixesDatabase.WorkerThreads = 1
HotfixesDatabase.SynchThreads  = 1
```

### Overlay Tables Schema

All dynamic race masks are stored as `VARBINARY` (byte arrays, Big-Endian in MySQL):

#### 1. Items (`dynamic_racemask_item`)
Overlays race restrictions checked during equipping/using items (`Player::CanUseItem`):
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_item` (
    `entry` INT UNSIGNED NOT NULL COMMENT 'Item template entry (item_template.entry)',
    `racemask` VARBINARY(32) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

#### 2. Quests (`dynamic_racemask_quest`)
Overlays race restrictions checked when viewing or accepting quests (`Player::SatisfyQuestRace`):
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_quest` (
    `entry` INT UNSIGNED NOT NULL COMMENT 'Quest ID (quest_template.ID)',
    `racemask` VARBINARY(32) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
    PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

#### 3. Conditions (`dynamic_racemask_condition`)
Overlays `CONDITION_RACE` (`16`) checks in `ConditionMgr`. Triggered when `ConditionValue2 == 1`, where `ConditionValue1` references `dynamic_racemask_condition.id`:
```sql
CREATE TABLE IF NOT EXISTS `dynamic_racemask_condition` (
    `id` INT UNSIGNED NOT NULL COMMENT 'Condition dynamic racemask ID (referenced by ConditionValue1 when ConditionValue2 = 1)',
    `racemask` VARBINARY(32) NOT NULL COMMENT 'Binary racemask bytes (Big-Endian in SQL, bit = raceId - 1)',
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

MySQL handles binary bitwise operators (`&`, `|`) in Big-Endian order:
- Race 1 only: `0x01`
- Race 35 only (bit 34): `0x0400000000` (5 bytes)
- Race 1 + Race 35: `0x0400000001`

---

## Tooling & Generation: `lua-dbc`

The module includes an automated Lua conversion script in `data/lua/generate_dynamic_mask.lua`.

It is powered by [**lua-dbc**](https://github.com/WarcraftXL-Labs/lua-dbc) - LuaJIT FFI bindings for World of Warcraft client databases.

### What `generate_dynamic_mask.lua` does:
1. Scans stock 3.3.5a DBC files (`SkillRaceClassInfo`, `SkillLineAbility`, `TalentTab`, `DanceMoves`, `Faction`).
2. Extracts and aggregates all distinct 32-bit `RaceMask` and `ClassMask` integers.
3. Generates `DynamicMask.dbc` with human-readable comments.
4. Rewrites the stock DBC files to replace legacy masks with `DynamicMask.dbc` IDs.
5. Preserves backup files (`.dbc.bak`) automatically.

### Running the Generator:
```bash
cd modules/mod-dynamic-mask/data/lua
luajit generate_dynamic_mask.lua --source /path/to/stock/dbc --output ../dbc
```

---

## Vanilla Data Migration (SQL)

A ready-to-run migration script is provided in `data/sql/db_hotfixes/migrate_vanilla_racemasks.sql`.

It performs the following operations:
1. **Items**: Migrates all items with race restrictions from `acore_world.item_template` into `dynamic_racemask_item` as binary bitmasks (skipping universal items with `AllowableRace = -1` or `0`).
2. **Quests**: Migrates all race-restricted quests from `acore_world.quest_template` into `dynamic_racemask_quest` (skipping universal quests with `AllowableRaces = 0`).
3. **Conditions**: Populates `dynamic_racemask_condition` with distinct legacy race masks from `acore_world.conditions` (setting `id` to the legacy mask value), then sets `ConditionValue2 = 1` in `conditions` to activate dynamic evaluation.

To execute manually:
```bash
mysql -u acore -p acore_hotfixes < modules/mod-dynamic-mask/data/sql/db_hotfixes/migrate_vanilla_racemasks.sql
```
(Or let AzerothCore's DBUpdater execute it automatically upon startup).

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

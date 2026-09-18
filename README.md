# mod-dynamic-mask: Dynamic Bitmask Extension for AzerothCore

## Overview

`mod-dynamic-mask` is an architectural module for AzerothCore (WoW 3.3.5a) that eliminates the engine's legacy 32-bit bitmask ceiling (`mask > 32`).

In standard 3.3.5a, bitmasks (such as race and class restrictions) are stored and evaluated as fixed `uint32` values ($1 \ll (\text{ID} - 1)$), capping identifiers to 32. This module introduces arbitrary-length bitmasks across all game layers—client/server DBCs, runtime SQL hotfixes, and network packets—without modifying stock Blizzard data.

---

## Architecture

```
                 ┌──────────────────────────────────────┐
                 │       Core (Vanilla / Unmodified)    │
                 │   MatchesRaceMask / MatchesClassMask │
                 └──────────────────┬───────────────────┘
                                    │ ScriptMgr Hooks
                 ┌──────────────────▼───────────────────┐
                 │          mod-dynamic-mask            │
                 ├──────────────────────────────────────┤
                 │ • WDBC Overlay (DynamicMasks.wdbc)   │◄── mod-wdbc
                 │ • Hotfixes DB  (acore_hotfixes)      │
                 │ • Wire Packets (Dynamic BitPack)     │
                 └──────────────────────────────────────┘
```

1. **DBC Overlay Layer (`DynamicMasks.wdbc`)**:
   Overlays extended bitmasks on top of vanilla DBC records (`SkillLineAbility`, `SkillRaceClassInfo`, `TalentTab`, `DanceMoves`, `Faction`) via `mod-wdbc`. All original Blizzard DBC files remain 100% vanilla.

2. **Hotfixes Database (`acore_hotfixes`)**:
   Hotfix overlay tables (`dynamic_racemask_item`, `dynamic_racemask_quest`, `dynamic_racemask_condition`) store arbitrary-length binary masks (`VARBINARY(32)`), reloadable at runtime without server downtime.

3. **Dynamic BitPack Network Streaming**:
   Length-prefixed binary stream (`wordCount` + 32-bit words) in `SMSG_ITEM_QUERY_SINGLE_RESPONSE` (`0x058`), allowing tooltips and equip validation for custom IDs with minimal packet payload.

---

## Core Hooks & Fallback

The core invokes two generic hooks via `MiscScript`:
- `OnCheckRaceMask(table, recordId, maskIndex, fallbackMask, race, result)`
- `OnCheckClassMask(table, recordId, maskIndex, fallbackMask, class_, result)`

If `mod-dynamic-mask` is not installed or an entry has no overlay, the core falls back to standard vanilla 32-bit evaluation:
$$\text{Fallback} = (\text{fallbackMask} \ \& \ (1 \ll (\text{ID} - 1))) \neq 0$$

---

## Database Configuration (`acore_hotfixes`)

Configured in `worldserver.conf`:
```ini
HotfixesDatabaseInfo = "127.0.0.1;3306;acore;acore;acore_hotfixes"
```

### Overlay Tables

- `dynamic_racemask_item` (`entry` INT, `racemask` VARBINARY(32))
- `dynamic_racemask_quest` (`entry` INT, `racemask` VARBINARY(32))
- `dynamic_racemask_condition` (`id` INT, `racemask` VARBINARY(32))

---

## In-Game Administration

Reloads both SQL tables and `DynamicMasks.wdbc` in memory on the fly:
```text
.reload dynamic_mask
```

---

## Documentation

- [DYNAMIC_MASK_DBC.md](DYNAMIC_MASK_DBC.md): Specification of the WDBC overlay format, table registry, and CSV dataset.
- [NETWORK_PROTOCOL.md](NETWORK_PROTOCOL.md): Specification of the Dynamic BitPack packet layout and client implementation.

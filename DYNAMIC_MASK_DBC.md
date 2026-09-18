# DynamicMasks.wdbc Overlay Specification

> **Container File:** `DynamicMasks.wdbc` (Managed via `mod-wdbc`)  
> **Source CSV:** `DynamicMasks.csv` (Converted bidirectionally via `wdbc_tool`)  
> **Stock Blizzard DBCs:** **100% Vanilla** — all original DBC files remain unmodified.

---

## 1. Overview

`DynamicMasks.wdbc` provides an external amendment layer for stock Blizzard DBC files. Instead of destructively altering DBC files with custom schemas or foreign keys, extended bitmasks are mapped to stock records using a composite 64-bit key:

$$\text{Key} = (\text{uint64}(\text{Table}) \ll 40) \mid (\text{uint64}(\text{MaskIndex}) \ll 32) \mid \text{uint64}(\text{RecordID})$$

---

## 2. Record Schema

| Index | Field | Type | Description |
|:-----:|:------|:----:|:------------|
| `0` | `Table` | `uint8` | Target DBC Table ID (see Registry below) |
| `1` | `RecordID` | `uint32` | Primary key `ID` in the target stock DBC |
| `2` | `MaskIndex` | `uint8` | Mask selector (`0` = RaceMask, `1` = ClassMask) |
| `3` | `Mask` | `blob` | Arbitrary-length binary bitmask (Little-Endian) |
| `4` | `Comment` | `string` | Maintenance description |

### Target Table Registry

| Table ID | DBC Table Name | Primary Key | Overlaid Masks |
|:--------:|:---------------|:------------|:---------------|
| `0` | **SkillLineAbility** | `SkillLineAbility.dbc` : `ID` | `0` = RaceMask, `1` = ClassMask |
| `1` | **SkillRaceClassInfo** | `SkillRaceClassInfo.dbc` : `ID` | `0` = RaceMask, `1` = ClassMask |
| `2` | **TalentTab** | `TalentTab.dbc` : `TalentTabID` | `0` = RaceMask, `1` = ClassMask |
| `3` | **DanceMoves** | `DanceMoves.dbc` : `ID` | `0` = RaceMask |
| `4` | **Faction** | `Faction.dbc` : `ID` | `0` = BaseRepRaceMask, `1` = BaseRepClassMask |

---

## 3. CSV Dataset Format (`DynamicMasks.csv`)

In the source CSV, the `Mask` column is represented as a hexadecimal byte string (`0x...`):

```csv
Table,RecordID,MaskIndex,Mask,Comment
0,13,0,0x00000001,RaceMask for SkillLineAbility 13
0,13,1,0x00000001,ClassMask for SkillLineAbility 13
1,1,0,0x0000044D,Alliance Races for SkillRaceClassInfo 1
2,1,0,0x00000000,Universal TalentTab 1
```

---

## 4. Compilation & Verification

Using `wdbc_tool` (provided by `mod-wdbc`):

```bash
# Compile CSV into binary container
wdbc_tool --to-wdbc DynamicMasks.csv DynamicMasks.wdbc

# Export binary container back to CSV
wdbc_tool --to-csv DynamicMasks.wdbc DynamicMasks.csv

# Inspect container metadata
wdbc_tool --info DynamicMasks.wdbc
```

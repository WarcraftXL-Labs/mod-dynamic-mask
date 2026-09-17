# DynamicMask.dbc - Complete Entry Reference

> **DBC Binary File:** `DynamicMask.dbc` (WDBC format for WoW 3.3.5a / WarcraftXL)
> **C++ Format String:** `char constexpr DynamicMaskfmt[] = "niss";`
> **Total Records:** 134 (76 RaceMask entries + 58 ClassMask entries)

---

## 1. DBC Structure (`DynamicMaskEntry`)

| Field | Index | C++ Type | DBC Format | Description |
| :--- | :---: | :--- | :---: | :--- |
| **ID** | 0 | `uint32` | `n` | Unique primary key used as a foreign key reference in stock DBCs. |
| **Type** | 1 | `uint32` | `i` | Bitmask type: `0` = RaceMask, `1` = ClassMask, `2+` = Reserved. |
| **Mask** | 2 | `char const*` | `s` | Hexadecimal bitmask string (e.g. `0x00000001`, arbitrary bit length). |
| **Comment** | 3 | `char const*` | `s` | Human-readable description of the races/classes covered by this mask. |

---

## 2. RaceMask Entries (`Type = 0` - 76 records)

These IDs replace legacy 32-bit `RaceMask` values in converted stock DBC files (e.g., `SkillRaceClassInfo.dbc`, `DanceMoves.dbc`, `Faction.dbc`).

| ID | Type | Hex Mask | Description / Included Races |
| :---: | :---: | :---: | :--- |
| **1** | `0` | `0x00000001` | Human |
| **2** | `0` | `0x00000002` | Orc |
| **3** | `0` | `0x00000004` | Dwarf |
| **4** | `0` | `0x00000006` | Orc, Dwarf |
| **5** | `0` | `0x00000008` | Night Elf |
| **6** | `0` | `0x00000010` | Undead |
| **7** | `0` | `0x00000020` | Tauren |
| **8** | `0` | `0x00000022` | Orc, Tauren |
| **9** | `0` | `0x00000024` | Dwarf, Tauren |
| **10** | `0` | `0x00000028` | Night Elf, Tauren |
| **11** | `0` | `0x00000040` | Gnome |
| **12** | `0` | `0x0000004D` | Vanilla Alliance (Human, Dwarf, Night Elf, Gnome) |
| **13** | `0` | `0x0000005F` | Human, Orc, Dwarf, Night Elf, Undead, Gnome |
| **14** | `0` | `0x00000080` | Troll |
| **15** | `0` | `0x00000082` | Orc, Troll |
| **16** | `0` | `0x000000A0` | Tauren, Troll |
| **17** | `0` | `0x000000A2` | Orc, Tauren, Troll |
| **18** | `0` | `0x000000A4` | Dwarf, Tauren, Troll |
| **19** | `0` | `0x000000A6` | Orc, Dwarf, Tauren, Troll |
| **20** | `0` | `0x000000A7` | Human, Orc, Dwarf, Tauren, Troll |
| **21** | `0` | `0x000000B2` | Vanilla Horde (Orc, Undead, Tauren, Troll) |
| **22** | `0` | `0x000000CF` | Human, Orc, Dwarf, Night Elf, Gnome, Troll |
| **23** | `0` | `0x000000D7` | Human, Orc, Dwarf, Undead, Gnome, Troll |
| **24** | `0` | `0x000000D8` | Night Elf, Undead, Gnome, Troll |
| **25** | `0` | `0x000000DB` | Human, Orc, Night Elf, Undead, Gnome, Troll |
| **26** | `0` | `0x000000DE` | Orc, Dwarf, Night Elf, Undead, Gnome, Troll |
| **27** | `0` | `0x000000FF` | All Vanilla Playable Races |
| **28** | `0` | `0x000001FD` | Human, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin |
| **29** | `0` | `0x00000200` | Blood Elf |
| **30** | `0` | `0x00000208` | Night Elf, Blood Elf |
| **31** | `0` | `0x00000210` | Undead, Blood Elf |
| **32** | `0` | `0x0000028A` | Orc, Night Elf, Troll, Blood Elf |
| **33** | `0` | `0x00000292` | Orc, Undead, Troll, Blood Elf |
| **34** | `0` | `0x000002A2` | Orc, Tauren, Troll, Blood Elf |
| **35** | `0` | `0x000002B2` | Horde (Orc, Undead, Tauren, Troll, Blood Elf) |
| **36** | `0` | `0x000003B2` | Orc, Undead, Tauren, Troll, Goblin, Blood Elf |
| **37** | `0` | `0x000003FF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf |
| **38** | `0` | `0x00000400` | Draenei |
| **39** | `0` | `0x00000405` | Human, Dwarf, Draenei |
| **40** | `0` | `0x0000040D` | Human, Dwarf, Night Elf, Draenei |
| **41** | `0` | `0x00000427` | Human, Orc, Dwarf, Tauren, Draenei |
| **42** | `0` | `0x00000445` | Human, Dwarf, Gnome, Draenei |
| **43** | `0` | `0x00000449` | Human, Night Elf, Gnome, Draenei |
| **44** | `0` | `0x0000044C` | Dwarf, Night Elf, Gnome, Draenei |
| **45** | `0` | `0x0000044D` | Alliance (Human, Dwarf, Night Elf, Gnome, Draenei) |
| **46** | `0` | `0x00000458` | Night Elf, Undead, Gnome, Draenei |
| **47** | `0` | `0x0000045B` | Human, Orc, Night Elf, Undead, Gnome, Draenei |
| **48** | `0` | `0x0000046D` | Human, Dwarf, Night Elf, Tauren, Gnome, Draenei |
| **49** | `0` | `0x000004A6` | Orc, Dwarf, Tauren, Troll, Draenei |
| **50** | `0` | `0x000004CD` | Human, Dwarf, Night Elf, Gnome, Troll, Draenei |
| **51** | `0` | `0x000005F9` | Human, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Draenei |
| **52** | `0` | `0x000005FF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Draenei |
| **53** | `0` | `0x00000608` | Night Elf, Blood Elf, Draenei |
| **54** | `0` | `0x0000068A` | Orc, Night Elf, Troll, Blood Elf, Draenei |
| **55** | `0` | `0x000006FF` | All Playable Races (Alliance + Horde) |
| **56** | `0` | `0x0000077F` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Goblin, Blood Elf, Draenei |
| **57** | `0` | `0x000007BF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Troll, Goblin, Blood Elf, Draenei |
| **58** | `0` | `0x000007DF` | Human, Orc, Dwarf, Night Elf, Undead, Gnome, Troll, Goblin, Blood Elf, Draenei |
| **59** | `0` | `0x000007EF` | Human, Orc, Dwarf, Night Elf, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei |
| **60** | `0` | `0x000007F7` | Human, Orc, Dwarf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei |
| **61** | `0` | `0x000007FB` | Human, Orc, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei |
| **62** | `0` | `0x000007FF` | All Playable Races and Goblin |
| **63** | `0` | `0x00000BFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Fel Orc |
| **64** | `0` | `0x00000FFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc |
| **65** | `0` | `0x00007BFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Fel Orc, Naga, Broken, Skeleton |
| **66** | `0` | `0x00007FFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton |
| **67** | `0` | `0x00027FFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Forest Troll |
| **68** | `0` | `0x0003FDFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll |
| **69** | `0` | `0x0003FF7F` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll |
| **70** | `0` | `0x0003FFDF` | Human, Orc, Dwarf, Night Elf, Undead, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll |
| **71** | `0` | `0x0003FFFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll |
| **72** | `0` | `0x0007FFFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll, Taunka |
| **73** | `0` | `0x001FFFFF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Fel Orc, Naga, Broken, Skeleton, Vrykul, Tuskarr, Forest Troll, Taunka, Northrend Skeleton, Ice Troll |
| **74** | `0` | `0x7FFFFFFF` | All Playable and Non-Playable Races (0x7FFFFFFF) |
| **75** | `0` | `0xFFFC07FF` | Human, Orc, Dwarf, Night Elf, Undead, Tauren, Gnome, Troll, Goblin, Blood Elf, Draenei, Taunka, Northrend Skeleton, Ice Troll, Race_22, Race_23, Race_24, Race_25, Race_26, Race_27, Race_28, Race_29, Race_30, Race_31 |
| **76** | `0` | `0xFFFFFFFF` | All Races (0xFFFFFFFF) |

---

## 3. ClassMask Entries (`Type = 1` - 58 records)

These IDs replace legacy 32-bit `ClassMask` values in converted stock DBC files (e.g., `SkillRaceClassInfo.dbc`, `SkillLineAbility.dbc`, `TalentTab.dbc`).

| ID | Type | Hex Mask | Description / Included Classes |
| :---: | :---: | :---: | :--- |
| **1001** | `1` | `0x00000001` | Warrior |
| **1002** | `1` | `0x00000002` | Paladin |
| **1003** | `1` | `0x00000003` | Warrior, Paladin |
| **1004** | `1` | `0x00000004` | Hunter |
| **1005** | `1` | `0x00000005` | Warrior, Hunter |
| **1006** | `1` | `0x00000008` | Rogue |
| **1007** | `1` | `0x00000009` | Warrior, Rogue |
| **1008** | `1` | `0x0000000C` | Hunter, Rogue |
| **1009** | `1` | `0x0000000D` | Warrior, Hunter, Rogue |
| **1010** | `1` | `0x0000000F` | Warrior, Paladin, Hunter, Rogue |
| **1011** | `1` | `0x00000010` | Priest |
| **1012** | `1` | `0x00000020` | Death Knight |
| **1013** | `1` | `0x00000023` | Warrior, Paladin, Death Knight |
| **1014** | `1` | `0x00000027` | Warrior, Paladin, Hunter, Death Knight |
| **1015** | `1` | `0x0000002D` | Warrior, Hunter, Rogue, Death Knight |
| **1016** | `1` | `0x00000040` | Shaman |
| **1017** | `1` | `0x00000043` | Warrior, Paladin, Shaman |
| **1018** | `1` | `0x00000044` | Hunter, Shaman |
| **1019** | `1` | `0x00000048` | Rogue, Shaman |
| **1020** | `1` | `0x0000004A` | Paladin, Rogue, Shaman |
| **1021** | `1` | `0x00000050` | Priest, Shaman |
| **1022** | `1` | `0x00000067` | Warrior, Paladin, Hunter, Death Knight, Shaman |
| **1023** | `1` | `0x0000006D` | Warrior, Hunter, Rogue, Death Knight, Shaman |
| **1024** | `1` | `0x0000006F` | Warrior, Paladin, Hunter, Rogue, Death Knight, Shaman |
| **1025** | `1` | `0x00000080` | Mage |
| **1026** | `1` | `0x00000090` | Priest, Mage |
| **1027** | `1` | `0x000000D0` | Priest, Shaman, Mage |
| **1028** | `1` | `0x00000100` | Warlock |
| **1029** | `1` | `0x00000108` | Rogue, Warlock |
| **1030** | `1` | `0x00000110` | Priest, Warlock |
| **1031** | `1` | `0x00000180` | Mage, Warlock |
| **1032** | `1` | `0x00000190` | Priest, Mage, Warlock |
| **1033** | `1` | `0x00000196` | Paladin, Hunter, Priest, Mage, Warlock |
| **1034** | `1` | `0x000001AF` | Warrior, Paladin, Hunter, Rogue, Death Knight, Mage, Warlock |
| **1035** | `1` | `0x000001FF` | Warrior, Paladin, Hunter, Rogue, Priest, Death Knight, Shaman, Mage, Warlock |
| **1036** | `1` | `0x00000400` | Druid |
| **1037** | `1` | `0x00000402` | Paladin, Druid |
| **1038** | `1` | `0x00000405` | Warrior, Hunter, Druid |
| **1039** | `1` | `0x00000407` | Warrior, Paladin, Hunter, Druid |
| **1040** | `1` | `0x00000408` | Rogue, Druid |
| **1041** | `1` | `0x00000427` | Warrior, Paladin, Hunter, Death Knight, Druid |
| **1042** | `1` | `0x0000043D` | Warrior, Hunter, Rogue, Priest, Death Knight, Druid |
| **1043** | `1` | `0x00000440` | Shaman, Druid |
| **1044** | `1` | `0x0000044D` | Warrior, Hunter, Rogue, Shaman, Druid |
| **1045** | `1` | `0x00000463` | Warrior, Paladin, Death Knight, Shaman, Druid |
| **1046** | `1` | `0x0000046F` | Warrior, Paladin, Hunter, Rogue, Death Knight, Shaman, Druid |
| **1047** | `1` | `0x0000047B` | Warrior, Paladin, Rogue, Priest, Death Knight, Shaman, Druid |
| **1048** | `1` | `0x0000049B` | Warrior, Paladin, Rogue, Priest, Mage, Druid |
| **1049** | `1` | `0x00000504` | Hunter, Warlock, Druid |
| **1050** | `1` | `0x0000057F` | Warrior, Paladin, Hunter, Rogue, Priest, Death Knight, Shaman, Warlock, Druid |
| **1051** | `1` | `0x000005D0` | Priest, Shaman, Mage, Warlock, Druid |
| **1052** | `1` | `0x000005D5` | Warrior, Hunter, Priest, Shaman, Mage, Warlock, Druid |
| **1053** | `1` | `0x000005DD` | Warrior, Hunter, Rogue, Priest, Shaman, Mage, Warlock, Druid |
| **1054** | `1` | `0x000005DF` | Warrior, Paladin, Hunter, Rogue, Priest, Shaman, Mage, Warlock, Druid |
| **1055** | `1` | `0x000005FF` | All Playable Classes (Warrior..Druid) |
| **1056** | `1` | `0x0003FFDF` | Warrior, Paladin, Hunter, Rogue, Priest, Shaman, Mage, Warlock, Class_10, Druid, Class_12, Class_13, Class_14, Class_15, Class_16, Class_17, Class_18 |
| **1057** | `1` | `0x0003FFFF` | Warrior, Paladin, Hunter, Rogue, Priest, Death Knight, Shaman, Mage, Warlock, Class_10, Druid, Class_12, Class_13, Class_14, Class_15, Class_16, Class_17, Class_18 |
| **1058** | `1` | `0xFFFFFFFF` | All Classes (0xFFFFFFFF) |

---

## 4. Converted Stock DBC Cross-Reference

| Stock DBC | Converted Field(s) | Type Used | Example Foreign Key Reference |
| :--- | :--- | :---: | :--- |
| `SkillRaceClassInfo.dbc` | `RaceMask`, `ClassMask` | Type 0 / Type 1 | `RaceMask = 45` (Alliance), `ClassMask = 1001` (Warrior) |
| `SkillLineAbility.dbc` | `RaceMask`, `ClassMask` | Type 0 / Type 1 | Direct foreign key lookup into `DynamicMask.dbc` |
| `TalentTab.dbc` | `RaceMask`, `ClassMask` | Type 0 / Type 1 | Restricts talent tree visibility by race/class combinations |
| `DanceMoves.dbc` | `RaceMask` | Type 0 | Assigns dance emote animations to specific races |
| `Faction.dbc` | `BaseRepRaceMask1..4` | Type 0 | Defines starting reputation standings per race |

---

## 5. Server-Side Mask Resolution & Fallback Logic

When checking race or class eligibility in core (`MatchesRaceMask`, `MatchesClassMask`):

```text
Read value (maskOrId) :
       │
       ├─── maskOrId == 0 ─────────────────────────► Allowed for all races/classes (true)
       │
       ├─── Found in sDynamicMaskCache[maskOrId] ──► DynamicBitMask::HasRace / HasClass
       │
       └─── Not found in cache ────────────────────► 32-bit fallback: (maskOrId & (1 << (id - 1))) != 0
```

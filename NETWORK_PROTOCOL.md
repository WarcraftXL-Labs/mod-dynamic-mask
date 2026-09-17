# Dynamic RaceMask Network Protocol Specification

This document defines the client $\leftrightarrow$ server network communication protocol for dynamic race bitmasks in AzerothCore and WarcraftXL clients (3.3.5a / 12340).

---

## 1. Background & Problem Statement

In the standard WoW 3.3.5a protocol, only one opcode transmits `AllowableRace` to the client for UI tooltips and equip validation:
- **`SMSG_ITEM_QUERY_SINGLE_RESPONSE`** (`0x058`).

In the vanilla protocol, `AllowableRace` is streamed as a fixed **4-byte unsigned integer** (`uint32`):
```text
[uint32 : AllowableClass]
[uint32 : AllowableRace]
[uint32 : ItemLevel]
```
This hard-limits playable race IDs to 1..32 (`1 << 31` max). To support custom race IDs $\ge 33$, the race mask field is upgraded to a self-describing **Dynamic BitPack**.

> **Note on Quests & Spells**: Quests (`SMSG_QUEST_QUERY_RESPONSE`) and Character Creation do **not** transmit race bitmasks over the network; quest validation is handled 100% server-side, and character creation uses single `uint8` race IDs. Only item queries require packet-level bitmask communication.

---

## 2. Dynamic BitPack Format

The Dynamic BitPack is a length-prefixed stream of 32-bit Little-Endian words:

```text
+---------------------+-------------------------+-------------------------+-----+
| uint8 : wordCount   | uint32 : word[0]        | uint32 : word[1]        | ... |
+---------------------+-------------------------+-------------------------+-----+
```

### Bit Mapping Rules
$$\text{Bit Index} = \text{RaceID} - 1$$
$$\text{Word Index} = \lfloor \frac{\text{RaceID} - 1}{32} \rfloor$$
$$\text{Word Bit} = (\text{RaceID} - 1) \pmod{32}$$

| Race ID | Bit Index | Target Word | Bit in Word | Mask in Word |
|:-------:|:---------:|:-----------:|:-----------:|:------------:|
| **1** (Human) | 0 | `word[0]` | 0 | `0x00000001` |
| **2** (Orc) | 1 | `word[0]` | 1 | `0x00000002` |
| **3** (Dwarf) | 2 | `word[0]` | 2 | `0x00000004` |
| **32** | 31 | `word[0]` | 31 | `0x80000000` |
| **33** (Custom) | 32 | `word[1]` | 0 | `0x00000001` |
| **35** (Custom) | 34 | `word[1]` | 2 | `0x00000004` |
| **64** | 63 | `word[1]` | 31 | `0x80000000` |
| **65** | 64 | `word[2]` | 0 | `0x00000001` |

### Length Optimization
- **`wordCount == 0`** (1 byte): Empty mask. No race restrictions (or unassigned).
- **`wordCount == 1`** (5 bytes): Races up to ID 32 (legacy range).
- **`wordCount == 2`** (9 bytes): Races up to ID 64.
- **`wordCount == 8`** (33 bytes): Max possible range for 255 race IDs (`uint8`).

---

## 3. Opcode Layout: `SMSG_ITEM_QUERY_SINGLE_RESPONSE` (`0x058`)

### Packet Stream Comparison

```text
=== VANILLA 3.3.5a ===                     === DYNAMIC BITPACK (WarcraftXL) ===
...                                        ...
uint32  InventoryType                      uint32  InventoryType
uint32  AllowableClass                     uint32  AllowableClass
uint32  AllowableRace  <-- 4 bytes         uint8   wordCount      <-- 1 byte
                                           uint32  word[0]        \
                                           [uint32 word[1] ...]    } N * 4 bytes
uint32  ItemLevel                          uint32  ItemLevel
uint32  RequiredLevel                      uint32  RequiredLevel
...                                        ...
```

---

## 4. Fallback Semantics

To prevent desynchronization between modded and unmodded environments, the server implements strict fallback rules:

### A. Item Has No Dynamic Mask Overlay
If an item has no entry in `acore_hotfixes.dynamic_racemask_item`:
The server packages the legacy `item_template.AllowableRace` value into the BitPack format:
- If `AllowableRace == 0`:
  - `wordCount = 0` (1 byte: `0x00`)
- If `AllowableRace != 0`:
  - `wordCount = 1`
  - `word[0] = AllowableRace` (5 bytes total: `0x01` followed by 4-byte LE `AllowableRace`)

This guarantees that the modded client parser always encounters a valid BitPack structure for every queried item.

### B. Module Disabled or Legacy Client Mode
If `DynamicMask.EnableItemOpcodeHook = 0` in `mod_dynamic_mask.conf` (or if the module is uninstalled):
The server bypasses the hook entirely and transmits the standard vanilla 4-byte `uint32 AllowableRace`.

---

## 5. Packet Byte Examples

### Example 1: Vanilla Item (Fallback) - Usable by Blood Elf (Race 10) only
- Race 10 $\rightarrow$ Bit 9 $\rightarrow$ `1 << 9 = 0x200` (`512`).
- No entry in `dynamic_racemask_item`.
- **Packet Bytes**:
  ```text
  01 00 02 00 00
  ^^ ^^^^^^^^^^^
  |  word[0] = 0x00000200 (Little-Endian)
  wordCount = 1
  ```

### Example 2: Dynamic Item - Usable by Human (1), Night Elf (4), and Custom Race 35
- Race 1 (Bit 0): `0x00000001` in `word[0]`
- Race 4 (Bit 3): `0x00000008` in `word[0]`
- Race 35 (Bit 34): `wordIndex = 1`, `bitInWord = 2` $\rightarrow$ `0x00000004` in `word[1]`
- SQL `dynamic_racemask_item`: `racemask = 0x0400000009` (Big-Endian in DB)
- **Packet Bytes**:
  ```text
  02 09 00 00 00 04 00 00 00
  ^^ ^^^^^^^^^^^ ^^^^^^^^^^^
  |  word[0]     word[1]
  wordCount = 2
  ```

### Example 3: Universal Item - No Race Restriction
- `wordCount = 0`
- **Packet Bytes**:
  ```text
  00
  ^^
  wordCount = 0 (0 words follow)
  ```

---

## 6. Client Implementation Reference

### C++ Client Unpacking (Game Client / Patch)

```cpp
struct DynamicBitMask
{
    uint8 wordCount = 0;
    std::vector<uint32> words;

    bool HasRace(uint32 raceId) const
    {
        if (raceId == 0)
            return false;

        uint32 const bit = raceId - 1;
        uint32 const wordIdx = bit / 32;
        uint32 const bitInWord = bit % 32;

        if (wordIdx >= words.size())
            return false;

        return (words[wordIdx] & (1U << bitInWord)) != 0;
    }
};

// In SMSG_ITEM_QUERY_SINGLE_RESPONSE packet handler:
DynamicBitMask itemRaceMask;
packet >> itemRaceMask.wordCount;
itemRaceMask.words.resize(itemRaceMask.wordCount);
for (uint8 i = 0; i < itemRaceMask.wordCount; ++i)
{
    packet >> itemRaceMask.words[i];
}
```

### Tooltip & Usability Check (C++ / Lua)

```cpp
bool CanPlayerEquipRace(DynamicBitMask const& mask, uint32 playerRaceId)
{
    // If mask is empty or all-1s (0xFFFFFFFF), no restriction
    if (mask.wordCount == 0)
        return true;

    // Check if player's specific race bit is enabled
    return mask.HasRace(playerRaceId);
}
```


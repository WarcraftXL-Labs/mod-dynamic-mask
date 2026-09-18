# Dynamic BitPack Network Protocol Specification

## 1. Overview

In World of Warcraft 3.3.5a (build 12340), `SMSG_ITEM_QUERY_SINGLE_RESPONSE` (`0x058`) transmits `AllowableRace` as a fixed 4-byte unsigned integer (`uint32`), physically capping race identifiers to 1..32 ($1 \ll 31$).

The **Dynamic BitPack** protocol replaces this fixed field with a self-describing, variable-length stream:

```text
+-------------------+--------------------+--------------------+-----+
| uint8 : wordCount | uint32 : word[0]   | uint32 : word[1]   | ... |
+-------------------+--------------------+--------------------+-----+
```

> **Note**: Quests (`SMSG_QUEST_QUERY_RESPONSE`) and Spells do not transmit race masks over the network. Their requirements are validated 100% server-side.

---

## 2. Bit Mapping Rules

$$\text{Bit Index} = \text{ID} - 1, \quad \text{Word Index} = \left\lfloor \frac{\text{ID} - 1}{32} \right\rfloor, \quad \text{Bit in Word} = (\text{ID} - 1) \pmod{32}$$

- **`wordCount == 0`** (1 byte): Universal (no restriction).
- **`wordCount == 1`** (5 bytes): Standard range (IDs 1..32).
- **`wordCount == 2`** (9 bytes): IDs 1..64.

---

## 3. Wire Layout (`SMSG_ITEM_QUERY_SINGLE_RESPONSE`)

```text
=== VANILLA PROTOCOL ===                 === DYNAMIC BITPACK ===
...                                      ...
uint32  AllowableClass                   uint32  AllowableClass
uint32  AllowableRace  <-- 4 bytes       uint8   wordCount      <-- 1 byte
                                         uint32  word[0]        \
                                         [uint32 word[1] ...]    } N * 4 bytes
uint32  ItemLevel                        uint32  ItemLevel
...                                      ...
```

---

## 4. Fallback Behavior
 
When an item has no custom entry in `dynamic_racemask_item`, the server serializes `item_template.AllowableRace` into the BitPack format (`wordCount = 0` if `0`, `wordCount = 1` otherwise) so patched clients always receive consistent data.

---

## 5. Client Unpacking Reference (C++)

```cpp
struct DynamicBitMask
{
    uint8 wordCount{0};
    std::vector<uint32> words;

    bool HasRace(uint32 raceId) const
    {
        if (raceId == 0)
            return false;

        uint32 const bit = raceId - 1;
        uint32 const wordIdx = bit / 32;
        if (wordIdx >= words.size())
            return false;

        return (words[wordIdx] & (1U << (bit % 32))) != 0;
    }
};

// In SMSG_ITEM_QUERY_SINGLE_RESPONSE handler:
DynamicBitMask itemRaceMask;
packet >> itemRaceMask.wordCount;
itemRaceMask.words.resize(itemRaceMask.wordCount);
for (uint8 i = 0; i < itemRaceMask.wordCount; ++i)
    packet >> itemRaceMask.words[i];
```

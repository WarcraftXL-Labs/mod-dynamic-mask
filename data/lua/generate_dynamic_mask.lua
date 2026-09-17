-- =============================================================================
--  __          __              _____                 __  __ _____ 
--  \ \        / /             / ____|          /\   |  \/  |  __ \
--   \ \  /\  / /_ _ _ __ ___ | |     _ __     /  \  | \  / | |__) |
--    \ \/  \/ / _` | '__/ __|| |    | '__|   / /\ \ | |\/| |  ___/ 
--     \  /\  / (_| | | | (__ | |____| |     / ____ \| |  | | |     
--      \/  \/ \__,_|_|  \___| \_____|_|    /_/    \_\_|  |_|_|     
--
--   DynamicMask.dbc Generator & Stock DBC Converter for AzerothCore
--   Breaking the 32-bit Race & Class Mask limits in WoW 3.3.5a (WarcraftXL)
--
--   Powered by lua-dbc:
--     GitHub:   https://github.com/WarcraftXL-Labs/lua-dbc
--     LuaRocks: https://luarocks.org/modules/WarcraftXL-Labs/lua-dbc
-- =============================================================================

local dbc = require("dbc")

print([[
=============================================================================
  mod-dynamic-mask :: DynamicMask.dbc Generator & DBC Converter
  Powered by lua-dbc (https://github.com/WarcraftXL-Labs/lua-dbc)
=============================================================================
]])

-- -----------------------------------------------------------------------------
-- Playable & Non-Playable Race Names (for human-readable mask descriptions)
-- -----------------------------------------------------------------------------
local races = {
    [1]  = "Human",
    [2]  = "Orc",
    [3]  = "Dwarf",
    [4]  = "Night Elf",
    [5]  = "Undead",
    [6]  = "Tauren",
    [7]  = "Gnome",
    [8]  = "Troll",
    [9]  = "Goblin",
    [10] = "Blood Elf",
    [11] = "Draenei",
    [12] = "Fel Orc",
    [13] = "Naga",
    [14] = "Broken",
    [15] = "Skeleton",
    [16] = "Vrykul",
    [17] = "Tuskarr",
    [18] = "Forest Troll",
    [19] = "Taunka",
    [20] = "Northrend Skeleton",
    [21] = "Ice Troll",
}

-- -----------------------------------------------------------------------------
-- Playable Class Names
-- -----------------------------------------------------------------------------
local classes = {
    [1]  = "Warrior",
    [2]  = "Paladin",
    [3]  = "Hunter",
    [4]  = "Rogue",
    [5]  = "Priest",
    [6]  = "Death Knight",
    [7]  = "Shaman",
    [8]  = "Mage",
    [9]  = "Warlock",
    [11] = "Druid",
}

-- -----------------------------------------------------------------------------
-- Helper: Describe Race Bitmask
-- -----------------------------------------------------------------------------
local function describe_race_mask(mask)
    local u = (mask < 0) and (mask + 4294967296) or mask
    if u == 0 then return "None" end
    if u == 4294967295 then return "All Races (0xFFFFFFFF)" end
    if u == 2147483647 then return "All Playable and Non-Playable Races (0x7FFFFFFF)" end
    if u == 1791 then return "All Playable Races (Alliance + Horde)" end
    if u == 2047 then return "All Playable Races and Goblin" end
    if u == 1101 then return "Alliance (Human, Dwarf, Night Elf, Gnome, Draenei)" end
    if u == 690 then return "Horde (Orc, Undead, Tauren, Troll, Blood Elf)" end
    if u == 77 then return "Vanilla Alliance (Human, Dwarf, Night Elf, Gnome)" end
    if u == 178 then return "Vanilla Horde (Orc, Undead, Tauren, Troll)" end
    if u == 255 then return "All Vanilla Playable Races" end

    local list = {}
    for id = 1, 31 do
        local bit_val = bit.lshift(1, id - 1)
        if bit.band(u, bit_val) ~= 0 then
            table.insert(list, races[id] or ("Race_" .. id))
        end
    end
    return #list > 0 and table.concat(list, ", ") or string.format("Mask(0x%08X)", u)
end

-- -----------------------------------------------------------------------------
-- Helper: Describe Class Bitmask
-- -----------------------------------------------------------------------------
local function describe_class_mask(mask)
    local u = (mask < 0) and (mask + 4294967296) or mask
    if u == 0 then return "None" end
    if u == 4294967295 then return "All Classes (0xFFFFFFFF)" end
    if u == 1535 then return "All Playable Classes (Warrior..Druid)" end

    local list = {}
    for id = 1, 31 do
        local bit_val = bit.lshift(1, id - 1)
        if bit.band(u, bit_val) ~= 0 then
            table.insert(list, classes[id] or ("Class_" .. id))
        end
    end
    return #list > 0 and table.concat(list, ", ") or string.format("Mask(0x%08X)", u)
end

-- -----------------------------------------------------------------------------
-- Parse CLI Arguments & Configure Directories
-- -----------------------------------------------------------------------------
local source_dir = nil
local output_dir = nil

local idx = 1
while idx <= #arg do
    if arg[idx] == "--source" and arg[idx + 1] then
        source_dir = arg[idx + 1]
        idx = idx + 2
    elseif arg[idx] == "--output" and arg[idx + 1] then
        output_dir = arg[idx + 1]
        idx = idx + 2
    else
        idx = idx + 1
    end
end

-- Default paths relative to module data/lua directory
if not source_dir then
    local candidates = {
        [[..\..\..\build\bin\Release\data\dbc]],
        [[..\dbc]],
        [[.\dbc]],
    }
    for _, path in ipairs(candidates) do
        local f = io.open(path .. "\\SkillLineAbility.dbc", "rb")
        if f then
            f:close()
            source_dir = path
            break
        end
    end
    source_dir = source_dir or [[..\..\..\build\bin\Release\data\dbc]]
end

if not output_dir then
    output_dir = [[..\dbc]]
end

print("Source DBC Directory: " .. source_dir)
print("Output DBC Directory: " .. output_dir)
print("")

-- -----------------------------------------------------------------------------
-- Step 1: Collect all unique RaceMask and ClassMask integers
-- -----------------------------------------------------------------------------
local race_set = {}
local class_set = {}

local function collect(name, race_fields, class_fields)
    local file_path = source_dir .. "\\" .. name .. ".dbc"
    local ok, tbl = pcall(dbc.Open, file_path, name, "3.3.5.12340")
    if not ok or not tbl then
        print("[WARN] Could not open " .. file_path)
        return
    end

    print(string.format("Scanning %-22s (%d rows)...", name .. ".dbc", tbl:Count()))

    for i = 1, tbl:Count() do
        local r = tbl:GetRowByIndex(i)
        for _, rf in ipairs(race_fields) do
            local val = r:GetField(rf)
            if val and val ~= 0 then
                local u = (val < 0) and (val + 4294967296) or val
                race_set[u] = true
            end
        end
        for _, cf in ipairs(class_fields) do
            local val = r:GetField(cf)
            if val and val ~= 0 then
                local u = (val < 0) and (val + 4294967296) or val
                class_set[u] = true
            end
        end
    end
end

collect("SkillRaceClassInfo", {"RaceMask"}, {"ClassMask"})
collect("SkillLineAbility",   {"RaceMask", "ExcludeRace"}, {"ClassMask", "ExcludeClass"})
collect("TalentTab",          {"RaceMask"}, {"ClassMask"})
collect("DanceMoves",         {"Racemask"}, {})

-- Faction array fields (ReputationRaceMask[4], ReputationClassMask[4])
local f_path = source_dir .. "\\Faction.dbc"
local f_ok, f_tbl = pcall(dbc.Open, f_path, "Faction", "3.3.5.12340")
if f_ok and f_tbl then
    print(string.format("Scanning %-22s (%d rows)...", "Faction.dbc", f_tbl:Count()))
    for i = 1, f_tbl:Count() do
        local r = f_tbl:GetRowByIndex(i)
        for arr_idx = 1, 4 do
            local rm = r:GetField("ReputationRaceMask", arr_idx)
            if rm and rm ~= 0 then
                local u = (rm < 0) and (rm + 4294967296) or rm
                race_set[u] = true
            end
            local cm = r:GetField("ReputationClassMask", arr_idx)
            if cm and cm ~= 0 then
                local u = (cm < 0) and (cm + 4294967296) or cm
                class_set[u] = true
            end
        end
    end
end

local sorted_r = {}
for u, _ in pairs(race_set) do table.insert(sorted_r, u) end
table.sort(sorted_r)

local sorted_c = {}
for u, _ in pairs(class_set) do table.insert(sorted_c, u) end
table.sort(sorted_c)

print(string.format("\nFound %d unique RaceMask values and %d unique ClassMask values.", #sorted_r, #sorted_c))

-- -----------------------------------------------------------------------------
-- Step 2: Build DynamicMask table (WDBC format: ID, Type, Mask, Comment)
-- -----------------------------------------------------------------------------
local dynamic_tbl = dbc.Create("DynamicMask", "WDBC", "3.3.5.12340")

-- Assign RaceMask entries (Type = 0, IDs 1..N)
local cur_id = 1
local race_id_map = {}
for _, u in ipairs(sorted_r) do
    local hex_str = string.format("0x%08X", u)
    local desc = describe_race_mask(u)
    dynamic_tbl:Create(cur_id, {
        Type    = 0,
        Mask    = hex_str,
        Comment = desc,
    })
    race_id_map[u] = cur_id
    cur_id = cur_id + 1
end

-- Assign ClassMask entries (Type = 1, IDs 1001..M)
cur_id = 1001
local class_id_map = {}
for _, u in ipairs(sorted_c) do
    local hex_str = string.format("0x%08X", u)
    local desc = describe_class_mask(u)
    dynamic_tbl:Create(cur_id, {
        Type    = 1,
        Mask    = hex_str,
        Comment = desc,
    })
    class_id_map[u] = cur_id
    cur_id = cur_id + 1
end

-- Save DynamicMask.dbc to output and source directories
local out_dm = output_dir .. "\\DynamicMask.dbc"
dynamic_tbl:Save(out_dm)
print(string.format("Generated DynamicMask.dbc -> %s (%d records)", out_dm, dynamic_tbl:Count()))

if source_dir ~= output_dir then
    pcall(function() dynamic_tbl:Save(source_dir .. "\\DynamicMask.dbc") end)
end

-- -----------------------------------------------------------------------------
-- Step 3: Rewrite stock DBC files to reference DynamicMask IDs
-- -----------------------------------------------------------------------------
local function convert_dbc(dbc_name, convert_fn)
    local src_file = source_dir .. "\\" .. dbc_name .. ".dbc"
    local bak_file = source_dir .. "\\" .. dbc_name .. ".dbc.bak"

    -- Create backup of original stock DBC if not present
    local f_test = io.open(bak_file, "rb")
    if f_test then
        f_test:close()
    else
        local f_in = io.open(src_file, "rb")
        if f_in then
            local data = f_in:read("*a")
            f_in:close()
            local f_bak = io.open(bak_file, "wb")
            if f_bak then
                f_bak:write(data)
                f_bak:close()
                print(string.format("  [BACKUP] Created %s.bak", dbc_name))
            end
        end
    end

    local tbl = dbc.Open(src_file, dbc_name, "3.3.5.12340")
    local count = tbl:Count()
    local converted = 0

    for i = 1, count do
        local r = tbl:GetRowByIndex(i)
        if convert_fn(r) then
            converted = converted + 1
        end
    end

    -- Save to output module DBC directory
    local target_file = output_dir .. "\\" .. dbc_name .. ".dbc"
    tbl:Save(target_file)

    -- If source differs from output, update source as well
    if source_dir ~= output_dir then
        pcall(function() tbl:Save(src_file) end)
    end

    print(string.format("  [CONVERT] %-20s: %d / %d records updated -> %s",
        dbc_name .. ".dbc", converted, count, target_file))
end

print("\nRewriting stock DBC files with DynamicMask foreign keys:")

-- 1. SkillRaceClassInfo.dbc
convert_dbc("SkillRaceClassInfo", function(r)
    local modified = false
    local rm = r:GetField("RaceMask")
    if rm and rm ~= 0 then
        local u = (rm < 0) and (rm + 4294967296) or rm
        if race_id_map[u] then
            r:SetField("RaceMask", race_id_map[u])
            modified = true
        end
    end

    local cm = r:GetField("ClassMask")
    if cm and cm ~= 0 then
        local u = (cm < 0) and (cm + 4294967296) or cm
        if class_id_map[u] then
            r:SetField("ClassMask", class_id_map[u])
            modified = true
        end
    end
    return modified
end)

-- 2. SkillLineAbility.dbc
convert_dbc("SkillLineAbility", function(r)
    local modified = false
    local rm = r:GetField("RaceMask")
    if rm and rm ~= 0 then
        local u = (rm < 0) and (rm + 4294967296) or rm
        if race_id_map[u] then
            r:SetField("RaceMask", race_id_map[u])
            modified = true
        end
    end

    local cm = r:GetField("ClassMask")
    if cm and cm ~= 0 then
        local u = (cm < 0) and (cm + 4294967296) or cm
        if class_id_map[u] then
            r:SetField("ClassMask", class_id_map[u])
            modified = true
        end
    end

    local er = r:GetField("ExcludeRace")
    if er and er ~= 0 then
        local u = (er < 0) and (er + 4294967296) or er
        if race_id_map[u] then
            r:SetField("ExcludeRace", race_id_map[u])
            modified = true
        end
    end

    local ec = r:GetField("ExcludeClass")
    if ec and ec ~= 0 then
        local u = (ec < 0) and (ec + 4294967296) or ec
        if class_id_map[u] then
            r:SetField("ExcludeClass", class_id_map[u])
            modified = true
        end
    end
    return modified
end)

-- 3. TalentTab.dbc
convert_dbc("TalentTab", function(r)
    local modified = false
    local rm = r:GetField("RaceMask")
    if rm and rm ~= 0 then
        local u = (rm < 0) and (rm + 4294967296) or rm
        if race_id_map[u] then
            r:SetField("RaceMask", race_id_map[u])
            modified = true
        end
    end

    local cm = r:GetField("ClassMask")
    if cm and cm ~= 0 then
        local u = (cm < 0) and (cm + 4294967296) or cm
        if class_id_map[u] then
            r:SetField("ClassMask", class_id_map[u])
            modified = true
        end
    end
    return modified
end)

-- 4. DanceMoves.dbc
convert_dbc("DanceMoves", function(r)
    local modified = false
    local rm = r:GetField("Racemask")
    if rm and rm ~= 0 then
        local u = (rm < 0) and (rm + 4294967296) or rm
        if race_id_map[u] then
            r:SetField("Racemask", race_id_map[u])
            modified = true
        end
    end
    return modified
end)

-- 5. Faction.dbc
convert_dbc("Faction", function(r)
    local modified = false
    for arr_idx = 1, 4 do
        local rm = r:GetField("ReputationRaceMask", arr_idx)
        if rm and rm ~= 0 then
            local u = (rm < 0) and (rm + 4294967296) or rm
            if race_id_map[u] then
                r:SetField("ReputationRaceMask", race_id_map[u], arr_idx)
                modified = true
            end
        end

        local cm = r:GetField("ReputationClassMask", arr_idx)
        if cm and cm ~= 0 then
            local u = (cm < 0) and (cm + 4294967296) or cm
            if class_id_map[u] then
                r:SetField("ReputationClassMask", class_id_map[u], arr_idx)
                modified = true
            end
        end
    end
    return modified
end)

print([[

=============================================================================
  Conversion completed successfully!
  Stock DBCs updated and DynamicMask.dbc generated.
  Visit https://github.com/WarcraftXL-Labs/lua-dbc for more info on lua-dbc.
=============================================================================
]])


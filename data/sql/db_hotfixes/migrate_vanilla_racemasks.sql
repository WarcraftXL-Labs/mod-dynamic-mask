-- -----------------------------------------------------------------------------
-- AzerothCore mod-dynamic-mask: Vanilla RaceMask Migration Script
-- -----------------------------------------------------------------------------
-- Purpose:
--   Migrates legacy 32-bit race bitmasks from `acore_world` tables into
--   the dynamic binary bitmask tables in `acore_hotfixes`.
--
-- Supported Tables:
--   - acore_world.item_template   -> acore_hotfixes.dynamic_racemask_item
--   - acore_world.quest_template  -> acore_hotfixes.dynamic_racemask_quest
--   - acore_world.conditions      -> acore_hotfixes.dynamic_racemask_condition
--
-- Notes:
--   1. If your world database is not named `acore_world`, replace all occurrences
--      of `acore_world` with your actual database name before running.
--   2. Items with AllowableRace = -1 (or 0) and quests with AllowableRaces = 0
--      are skipped to preserve core fallback for universal items/quests.
-- -----------------------------------------------------------------------------

-- -----------------------------------------------------------------------------
-- 1. Migrate item race restrictions
-- -----------------------------------------------------------------------------
-- Only items with specific race restrictions (AllowableRace > 0 and not -1/all-allowed)
-- are migrated into dynamic_racemask_item.
-- -----------------------------------------------------------------------------
INSERT INTO `dynamic_racemask_item` (`entry`, `racemask`)
SELECT
    `entry`,
    UNHEX(LPAD(HEX(`AllowableRace`), 8, '0')) AS `racemask`
FROM `acore_world`.`item_template`
WHERE `AllowableRace` > 0
  AND `AllowableRace` NOT IN (-1, 4294967295, 2147483647)
ON DUPLICATE KEY UPDATE `racemask` = VALUES(`racemask`);

-- -----------------------------------------------------------------------------
-- 2. Migrate quest race restrictions
-- -----------------------------------------------------------------------------
-- Only quests with specific race restrictions (AllowableRaces > 0)
-- are migrated into dynamic_racemask_quest.
-- -----------------------------------------------------------------------------
INSERT INTO `dynamic_racemask_quest` (`entry`, `racemask`)
SELECT
    `ID` AS `entry`,
    UNHEX(LPAD(HEX(`AllowableRaces`), 8, '0')) AS `racemask`
FROM `acore_world`.`quest_template`
WHERE `AllowableRaces` > 0
  AND `AllowableRaces` NOT IN (4294967295)
ON DUPLICATE KEY UPDATE `racemask` = VALUES(`racemask`);

-- -----------------------------------------------------------------------------
-- 3. Populate dynamic_racemask_condition for existing CONDITION_RACE entries
-- -----------------------------------------------------------------------------
-- For each distinct race mask used in `conditions` (ConditionTypeOrReference = 16),
-- inserts a row into dynamic_racemask_condition. The ID matches the legacy bitmask value,
-- allowing ConditionValue1 to remain unchanged while activating dynamic mode.
-- -----------------------------------------------------------------------------
INSERT INTO `dynamic_racemask_condition` (`id`, `racemask`)
SELECT DISTINCT
    `ConditionValue1` AS `id`,
    UNHEX(LPAD(HEX(`ConditionValue1`), 8, '0')) AS `racemask`
FROM `acore_world`.`conditions`
WHERE `ConditionTypeOrReference` = 16 -- CONDITION_RACE
  AND `ConditionValue1` > 0
ON DUPLICATE KEY UPDATE `racemask` = VALUES(`racemask`);

-- -----------------------------------------------------------------------------
-- 4. Activate dynamic mode on existing conditions
-- -----------------------------------------------------------------------------
-- Sets ConditionValue2 = 1 on CONDITION_RACE entries, signaling the server
-- that ConditionValue1 references dynamic_racemask_condition.id.
-- -----------------------------------------------------------------------------
UPDATE IGNORE `acore_world`.`conditions`
SET `ConditionValue2` = 1
WHERE `ConditionTypeOrReference` = 16 -- CONDITION_RACE
  AND `ConditionValue1` > 0
  AND `ConditionValue2` = 0;


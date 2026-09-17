-- Table structure for dynamic_racemask_item
CREATE TABLE IF NOT EXISTS `dynamic_racemask_item` (
  `entry` int unsigned NOT NULL COMMENT 'Item entry (matches item_template.entry)',
  `racemask` varbinary(32) NOT NULL COMMENT 'Dynamic race bitmask in Big-Endian binary format',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Dynamic racemask overlay for item_template';

-- Table structure for dynamic_racemask_quest
CREATE TABLE IF NOT EXISTS `dynamic_racemask_quest` (
  `entry` int unsigned NOT NULL COMMENT 'Quest entry (matches quest_template.ID)',
  `racemask` varbinary(32) NOT NULL COMMENT 'Dynamic race bitmask in Big-Endian binary format',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Dynamic racemask overlay for quest_template';

-- Table structure for dynamic_racemask_condition
CREATE TABLE IF NOT EXISTS `dynamic_racemask_condition` (
  `id` int unsigned NOT NULL COMMENT 'Condition overlay ID (matches conditions.ConditionValue1 when ConditionValue2 = 1)',
  `racemask` varbinary(32) NOT NULL COMMENT 'Dynamic race bitmask in Big-Endian binary format',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Dynamic racemask overlay for conditions';

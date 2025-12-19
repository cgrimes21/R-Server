# RTK Database Migration Scripts

This document describes each database migration script in the `scripts/` folder.

## How Migrations Work

Each script uses a stored procedure pattern that:
1. Checks if the script has already been run (via `MigrationHistory` table)
2. Executes the changes if not already applied
3. Records itself in `MigrationHistory` to prevent duplicate runs

This makes all scripts **idempotent** - safe to run multiple times.

---

## Required Scripts (Run in Order)

These three scripts must be run first to set up a functional database:

### 01_CreateRtkDatabaseAndUser.sql
**Category:** Setup
**Required:** Yes

Creates the foundation:
- Creates the `RTK` database
- Creates the `rtk` MySQL user with password `50LM8U8Poq5uX2AZJVKs`
- Grants all privileges on RTK database to the rtk user
- Creates the `MigrationHistory` table for tracking migrations

```bash
sudo mysql < scripts/01_CreateRtkDatabaseAndUser.sql
```

---

### 02_CreateTables.sql
**Category:** Setup
**Required:** Yes
**Size:** ~2MB (contains game data)

Creates all 52 database tables with initial game data:
- Player tables: `Accounts`, `Character`, `Inventory`, `Equipment`, `Banks`, etc.
- Game world: `Maps`, `Warps`, `Paths`, `NPCs0`, `Mobs`, `Spawns0/1`
- Items & spells: `Items`, `ItemSets`, `Recipes`, `Spells`, `SpellBook`
- Registries: `Registry`, `QuestRegistry`, `GameRegistry0`, `MapRegistry`, `NPCRegistry`
- Social: `Friends`, `Mail`, `Parcels`, `Clans`, `Boards`
- System: `Time`, `UpTime`, `Maintenance`, `BannedIP`

Includes INSERT statements for all game content (maps, items, mobs, NPCs, warps, etc.)

```bash
sudo mysql RTK < scripts/02_CreateTables.sql
```

---

### 03_FixErrors.sql
**Category:** Fix
**Required:** Yes

Fixes issues with the initial data:
- Inserts two reserved characters (`Reserved1`, `Reserved2`) - required for proper ID sequencing
- Fixes typo in "Foraged Fields 1" map name (had leading space)
- Moves NPCs from non-existent map 7200 to test map 1071

```bash
sudo mysql RTK < scripts/03_FixErrors.sql
```

---

## Optional Scripts

These scripts add content, fix bugs, or rebalance game mechanics. Run them in order if desired.

---

### 04_ReduceMobVitaAndExp.sql
**Category:** Balance
**Size:** Large (~60K tokens)

Reduces HP (Vita) and Experience values for all monsters in the game. This is a major balance pass that affects every mob in the `Mobs` table.

**Effect:** Makes leveling faster and mobs easier to kill.

---

### 05_DoNotRequireAccountPassword.sql
**Category:** Authentication

Modifies the `Accounts` table to allow empty passwords:
```sql
ALTER TABLE `Accounts` MODIFY `AccountPassword` varchar(255) NOT NULL DEFAULT '';
```

**Effect:** Accounts can be created without passwords (useful for development/testing).

---

### 06_AddTigerSentries.sql
**Category:** Content

Adds three new Tiger-themed monsters to the `Mobs` table:

| MobId | Name | Level | HP | Exp |
|-------|------|-------|-----|-----|
| 804 | Tiger sentry | 31 | 278,500 | 1,675,000 |
| 805 | Tiger guardian | 32 | 1,075,000 | 7,850,000 |
| 806 | Tiger defender | 33 | 5,550,000 | 10,000,000 |

---

### 07_FixRoosterGuardianSpawnTime.sql
**Category:** Fix

Sets the Rooster Guardian mob's spawn time to 30 seconds:
```sql
UPDATE Mobs SET MobSpawnTime = 30 WHERE MobIdentifier = 'rooster_guardian';
```

---

### 08_FixKeyToPondIcon.sql
**Category:** Fix

Corrects the item icon for "Key to Pond":
```sql
UPDATE Items SET ItmIcon = 485 WHERE ItmIdentifier = 'key_to_pond';
```

---

### 09_FixNpcIdentifierCasing.sql
**Category:** Fix

Converts ~190 NPC identifiers from `snake_case` to `PascalCase`. This aligns with Lua conventions where NPC identifiers function as global variables.

Examples:
- `clan_npc` → `ClanNpc`
- `merchant_npc` → `MerchantNpc`
- `warrior_trainer` → `WarriorTrainerNpc`
- `potion_shop` → `PotionShopNpc`

---

### 10_CreateKanPaymentsTable.sql
**Category:** Feature

Creates the `KanPayments` table for tracking premium currency (Kan) transactions:

| Column | Type | Description |
|--------|------|-------------|
| Id | varchar(50) | Payment ID |
| Status | varchar(20) | Payment status |
| Total | decimal(12,2) | Total amount |
| CustomerName | varchar(20) | Player name |
| CustomerEmail | varchar(100) | Email address |
| PaymentCurrency | varchar(10) | Currency code |
| ReceivedAmount | decimal(12,2) | Amount received |
| HasProcessed | tinyint(1) | Processing flag |
| ... | ... | Additional payment fields |

---

### 11_RebalanceWeapons.sql
**Category:** Balance

Rebalances weapon statistics across all weapon items. Adjusts damage, hit rating, stat bonuses, and level requirements.

---

### 12_RebalanceShields.sql
**Category:** Balance

Rebalances shield statistics. Adjusts AC (armor class), stat bonuses, and level requirements for all shield items.

---

### 13_RebalanceSubAccessories.sql
**Category:** Balance

Rebalances sub-accessory slot items (secondary accessories). Adjusts stat bonuses and level requirements.

---

### 14_RebalanceHelms.sql
**Category:** Balance

Rebalances helm/helmet items. Adjusts AC, stat bonuses, and level requirements.

---

### 15_RebalanceHandItems.sql
**Category:** Balance

Rebalances hand slot items (gloves, gauntlets, etc.). Adjusts stats and requirements.

---

### 16_InsertCotwGiasomoBird.sql
**Category:** Content

Adds a "Creature of the Week" mob - the Giasomo Bird:
- Creates MobId 807 based on mob 562
- Sets identifier to `cotw_giasomo_bird`
- Uses look 58 with default color

---

### 17_HideAbyssalCrystalNpcs.sql
**Category:** Content

Hides Abyssal Crystal NPCs by moving them to an unused map:
```sql
UPDATE NPCs0 SET NpcMapId = 1071, NpcX = 9, NpcY = 9 WHERE NpcId BETWEEN 364 AND 379;
```

**Effect:** Removes 16 NPCs from the game world without deleting them.

---

### 18_UpdateItems.sql
**Category:** Balance

Updates various item statistics. A general-purpose item balance pass.

---

### 19_InsertWaypointNpcs.sql
**Category:** Content

Adds three new waypoint/teleporter NPCs:

| NpcId | Identifier | Name | Location |
|-------|------------|------|----------|
| 392 | InnNpc | Cithra | Map 3812 (19,4) |
| 393 | NoxhilNpc | Noxhil | Map 1031 (5,4) |
| 394 | StelsiNpc | Stelsi | Map 522 (6,2) |

---

### 20_DuplicateHorseSentries.sql
**Category:** Content

Creates duplicate Horse Sentry mobs for additional spawn locations.

---

### 21_InsertMagisBanePoet.sql
**Category:** Content

Adds the "Magi's Bane" spell for the Poet class:
- Creates spell 4729 based on spell 3167
- Sets identifier to `magis_bane_poet`
- Assigns to Path 4 (Poet)
- Fixes capitalization: "Magi's bane" → "Magi's Bane"

---

## Running All Migrations

To apply all migrations in order:

```bash
cd /path/to/RTK-Server/database/scripts

# Required
sudo mysql < 01_CreateRtkDatabaseAndUser.sql
sudo mysql RTK < 02_CreateTables.sql
sudo mysql RTK < 03_FixErrors.sql

# Optional (run all or pick what you need)
for script in 0{4,5,6,7,8,9}_*.sql 1*.sql 2*.sql; do
    echo "Running $script..."
    sudo mysql RTK < "$script"
done
```

Or run them individually as needed.

---

## Checking Migration Status

To see which migrations have been applied:

```sql
SELECT Script, Timestamp FROM MigrationHistory ORDER BY Id;
```

---

## Creating New Migrations

Follow this template for new migration scripts:

```sql
USE RTK;

SET @script = 'XX_YourMigrationName.sql';

DELIMITER $$

DROP PROCEDURE IF EXISTS sp $$

CREATE PROCEDURE sp(IN scriptName VARCHAR(255))
BEGIN
    IF NOT EXISTS (SELECT * FROM MigrationHistory WHERE Script = scriptName) THEN
        -- Your changes here

        INSERT INTO `MigrationHistory` (Script,Timestamp) VALUES (scriptName,NOW());
    ELSE
        SELECT CONCAT(scriptName, ' was skipped because it is already present in the migration history.') AS '';
    END IF;
END $$

CALL sp(@script) $$

DROP PROCEDURE IF EXISTS sp $$

DELIMITER ;
```

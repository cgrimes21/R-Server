-- ============================================================================
-- RTK Server Database Initialization Script
-- ============================================================================
-- This script sets up a fresh RTK database with all required tables and data.
--
-- RECOMMENDED: Use setup.sh instead, which reads credentials from db.conf:
--   ./setup.sh init
--
-- Manual usage (uses hardcoded credentials below):
--   mysql -u root -p < rtk_init.sql
--
-- IMPORTANT: The credentials below must match db.conf and rtk/conf/char.conf
-- To change credentials, edit db.conf and run: ./setup.sh init
-- ============================================================================

-- ----------------------------------------------------------------------------
-- Step 1: Create Database
-- ----------------------------------------------------------------------------
CREATE DATABASE IF NOT EXISTS `RTK` CHARACTER SET latin1;
USE `RTK`;

SELECT 'Created database RTK' AS Status;

-- ----------------------------------------------------------------------------
-- Step 2: Create User and Grant Privileges
-- ----------------------------------------------------------------------------
-- User: rtk
-- Password: 50LM8U8Poq5uX2AZJVKs (matches char.conf)
-- Note: In production, change this password!

-- Drop user if exists (ignore error if not exists)
DROP USER IF EXISTS 'rtk'@'%';
DROP USER IF EXISTS 'rtk'@'localhost';

-- Create user with both % and localhost access
CREATE USER 'rtk'@'%' IDENTIFIED BY '50LM8U8Poq5uX2AZJVKs';
CREATE USER 'rtk'@'localhost' IDENTIFIED BY '50LM8U8Poq5uX2AZJVKs';

-- Grant privileges
GRANT ALL PRIVILEGES ON RTK.* TO 'rtk'@'%';
GRANT ALL PRIVILEGES ON RTK.* TO 'rtk'@'localhost';
FLUSH PRIVILEGES;

SELECT 'Created user rtk with privileges' AS Status;

-- ----------------------------------------------------------------------------
-- Step 3: Create Migration History Table
-- ----------------------------------------------------------------------------
DROP TABLE IF EXISTS `MigrationHistory`;
CREATE TABLE `MigrationHistory` (
  `Id` int(10) NOT NULL AUTO_INCREMENT,
  `Script` varchar(255) NOT NULL,
  `Timestamp` datetime NOT NULL,
  PRIMARY KEY (`Id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `MigrationHistory` (Script, Timestamp) VALUES ('rtk_init.sql', NOW());

SELECT 'Created MigrationHistory table' AS Status;

-- ----------------------------------------------------------------------------
-- Step 4: Create Core Tables
-- ----------------------------------------------------------------------------

-- Accounts
DROP TABLE IF EXISTS `Accounts`;
CREATE TABLE `Accounts` (
  `AccountId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AccountEmail` varchar(255) NOT NULL DEFAULT '',
  `AccountPassword` varchar(255) NOT NULL,
  `AccountKanBalance` int(10) NOT NULL DEFAULT '0',
  `AccountBanned` int(10) unsigned NOT NULL DEFAULT '0',
  `AccountCharId1` int(10) unsigned DEFAULT NULL,
  `AccountCharId2` int(10) unsigned DEFAULT NULL,
  `AccountCharId3` int(10) unsigned DEFAULT NULL,
  `AccountCharId4` int(10) unsigned DEFAULT NULL,
  `AccountCharId5` int(10) unsigned DEFAULT NULL,
  `AccountCharId6` int(10) unsigned DEFAULT NULL,
  `AccountConfirm` varchar(255) NOT NULL DEFAULT '',
  `AccountActivated` int(10) unsigned NOT NULL DEFAULT '0',
  `AccountTempPassword` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`AccountId`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

-- Character
DROP TABLE IF EXISTS `Character`;
CREATE TABLE `Character` (
  `ChaId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ChaName` varchar(16) NOT NULL DEFAULT '',
  `ChaPassword` varchar(16) NOT NULL DEFAULT '',
  `ChaAccountId` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaPath` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ChaLook` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaFace` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaHair` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaHairColor` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaLevel` int(10) unsigned NOT NULL DEFAULT '1',
  `ChaExperience` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ChaVita` int(10) unsigned NOT NULL DEFAULT '100',
  `ChaMaxVita` int(10) unsigned NOT NULL DEFAULT '100',
  `ChaMana` int(10) unsigned NOT NULL DEFAULT '100',
  `ChaMaxMana` int(10) unsigned NOT NULL DEFAULT '100',
  `ChaMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaX` int(10) unsigned NOT NULL DEFAULT '50',
  `ChaY` int(10) unsigned NOT NULL DEFAULT '50',
  `ChaGold` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ChaMight` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaGrace` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaWill` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaStats` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaClanId` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaClanRank` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ChaTitle` varchar(64) NOT NULL DEFAULT '',
  `ChaSpouse` varchar(16) NOT NULL DEFAULT '',
  `ChaGMLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ChaLastLogin` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaCreated` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaPlayTime` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaNation` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ChaLegendSlots` tinyint(3) unsigned NOT NULL DEFAULT '3',
  `ChaDeaths` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaKills` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaMobKills` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaAlignGood` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaAlignEvil` int(10) unsigned NOT NULL DEFAULT '0',
  `ChaFlags` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ChaId`),
  KEY `ChaName` (`ChaName`),
  KEY `ChaAccountId` (`ChaAccountId`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

-- Insert reserved characters (required for proper ID sequencing)
INSERT INTO `Character` (`ChaName`, `ChaPassword`) VALUES ('Reserved1', 'rsvd1');
INSERT INTO `Character` (`ChaName`, `ChaPassword`) VALUES ('Reserved2', 'rsvd2');

-- AdminPassword
DROP TABLE IF EXISTS `AdminPassword`;
CREATE TABLE `AdminPassword` (
  `AdmId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AdmActId` int(10) unsigned NOT NULL DEFAULT '0',
  `AdmPassword` varchar(255) NOT NULL DEFAULT '',
  `AdmTimer` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`AdmId`),
  KEY `AdmActId` (`AdmActId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Inventory
DROP TABLE IF EXISTS `Inventory`;
CREATE TABLE `Inventory` (
  `InvId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `InvChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `InvItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `InvAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `InvDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `InvEngrave` varchar(64) NOT NULL DEFAULT '',
  `InvTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `InvCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `InvCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `InvCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `InvCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `InvProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`InvId`),
  KEY `InvChaId` (`InvChaId`),
  KEY `InvItmId` (`InvItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Equipment
DROP TABLE IF EXISTS `Equipment`;
CREATE TABLE `Equipment` (
  `EqpId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `EqpChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpSlot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `EqpItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpEngrave` varchar(64) NOT NULL DEFAULT '',
  `EqpTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `EqpProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`EqpId`),
  KEY `EqpChaId` (`EqpChaId`),
  KEY `EqpItmId` (`EqpItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Banks
DROP TABLE IF EXISTS `Banks`;
CREATE TABLE `Banks` (
  `BnkId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `BnkChaId` int(11) unsigned NOT NULL DEFAULT '0',
  `BnkItmId` int(11) unsigned NOT NULL DEFAULT '0',
  `BnkDura` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkAmount` int(11) unsigned NOT NULL DEFAULT '0',
  `BnkEngrave` varchar(64) NOT NULL DEFAULT '',
  `BnkTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `BnkProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BnkId`),
  KEY `BnkChaId` (`BnkChaId`),
  KEY `BnkItmId` (`BnkItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Friends
DROP TABLE IF EXISTS `Friends`;
CREATE TABLE `Friends` (
  `FrdId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `FrdChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `FrdName` varchar(16) NOT NULL DEFAULT '',
  PRIMARY KEY (`FrdId`),
  KEY `FrdChaId` (`FrdChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Clans
DROP TABLE IF EXISTS `Clans`;
CREATE TABLE `Clans` (
  `ClanId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ClanName` varchar(32) NOT NULL DEFAULT '',
  `ClanNation` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ClanGold` bigint(20) unsigned NOT NULL DEFAULT '0',
  `ClanHall` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ClanId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ClanBanks
DROP TABLE IF EXISTS `ClanBanks`;
CREATE TABLE `ClanBanks` (
  `CbnkId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `CbnkClanId` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `CbnkDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkEngrave` varchar(64) NOT NULL DEFAULT '',
  `CbnkTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `CbnkProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`CbnkId`),
  KEY `CbnkClanId` (`CbnkClanId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Mail
DROP TABLE IF EXISTS `Mail`;
CREATE TABLE `Mail` (
  `MailId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MailChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `MailSender` varchar(16) NOT NULL DEFAULT '',
  `MailSubject` varchar(64) NOT NULL DEFAULT '',
  `MailBody` text,
  `MailRead` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MailDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`MailId`),
  KEY `MailChaId` (`MailChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Parcels
DROP TABLE IF EXISTS `Parcels`;
CREATE TABLE `Parcels` (
  `PrcId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `PrcChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcSender` varchar(16) NOT NULL DEFAULT '',
  `PrcItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `PrcDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcEngrave` varchar(64) NOT NULL DEFAULT '',
  `PrcTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcProtected` int(10) unsigned NOT NULL DEFAULT '0',
  `PrcDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`PrcId`),
  KEY `PrcChaId` (`PrcChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Legends
DROP TABLE IF EXISTS `Legends`;
CREATE TABLE `Legends` (
  `LegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `LegChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `LegSlot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `LegIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `LegColor` int(10) unsigned NOT NULL DEFAULT '0',
  `LegText` varchar(64) NOT NULL DEFAULT '',
  `LegDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`LegId`),
  KEY `LegChaId` (`LegChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- SpellBook
DROP TABLE IF EXISTS `SpellBook`;
CREATE TABLE `SpellBook` (
  `SbkId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SbkChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `SbkSplId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SbkId`),
  KEY `SbkChaId` (`SbkChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Aethers
DROP TABLE IF EXISTS `Aethers`;
CREATE TABLE `Aethers` (
  `AthId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AthChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `AthSplId` int(10) unsigned NOT NULL DEFAULT '0',
  `AthDuration` int(10) unsigned NOT NULL DEFAULT '0',
  `AthAether` int(10) unsigned NOT NULL DEFAULT '0',
  `AthPosition` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`AthId`),
  KEY `AthSplId` (`AthSplId`),
  KEY `AthChaId` (`AthChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Registry (Character-specific key-value storage)
DROP TABLE IF EXISTS `Registry`;
CREATE TABLE `Registry` (
  `RegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `RegChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `RegKey` varchar(64) NOT NULL DEFAULT '',
  `RegValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`RegId`),
  KEY `RegChaId` (`RegChaId`),
  KEY `RegKey` (`RegKey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- RegistryString
DROP TABLE IF EXISTS `RegistryString`;
CREATE TABLE `RegistryString` (
  `RegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `RegChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `RegKey` varchar(64) NOT NULL DEFAULT '',
  `RegValue` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`RegId`),
  KEY `RegChaId` (`RegChaId`),
  KEY `RegKey` (`RegKey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- QuestRegistry
DROP TABLE IF EXISTS `QuestRegistry`;
CREATE TABLE `QuestRegistry` (
  `QRegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `QRegChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `QRegQuestId` int(10) unsigned NOT NULL DEFAULT '0',
  `QRegKey` varchar(64) NOT NULL DEFAULT '',
  `QRegValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`QRegId`),
  KEY `QRegChaId` (`QRegChaId`),
  KEY `QRegQuestId` (`QRegQuestId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- GameRegistry0 (Global game state)
DROP TABLE IF EXISTS `GameRegistry0`;
CREATE TABLE `GameRegistry0` (
  `GRegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `GRegKey` varchar(64) NOT NULL DEFAULT '',
  `GRegValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`GRegId`),
  KEY `GRegKey` (`GRegKey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- MapRegistry
DROP TABLE IF EXISTS `MapRegistry`;
CREATE TABLE `MapRegistry` (
  `MRegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MRegMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `MRegKey` varchar(64) NOT NULL DEFAULT '',
  `MRegValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`MRegId`),
  KEY `MRegMapId` (`MRegMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- NPCRegistry
DROP TABLE IF EXISTS `NPCRegistry`;
CREATE TABLE `NPCRegistry` (
  `NRegId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `NRegNpcId` int(10) unsigned NOT NULL DEFAULT '0',
  `NRegKey` varchar(64) NOT NULL DEFAULT '',
  `NRegValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`NRegId`),
  KEY `NRegNpcId` (`NRegNpcId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Authorize
DROP TABLE IF EXISTS `Authorize`;
CREATE TABLE `Authorize` (
  `AutId` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `AutChaId` int(11) unsigned NOT NULL DEFAULT '0',
  `AutChaName` varchar(16) NOT NULL DEFAULT '',
  `AutIP` int(11) unsigned NOT NULL DEFAULT '0',
  `AutTimer` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`AutId`),
  KEY `AutChaName` (`AutChaName`),
  KEY `AutChaId` (`AutChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- BannedIP
DROP TABLE IF EXISTS `BannedIP`;
CREATE TABLE `BannedIP` (
  `BanId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BanIP` varchar(16) NOT NULL DEFAULT '',
  `BanExpire` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BanId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Kills
DROP TABLE IF EXISTS `Kills`;
CREATE TABLE `Kills` (
  `KillId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `KillChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `KillVictim` varchar(16) NOT NULL DEFAULT '',
  `KillDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`KillId`),
  KEY `KillChaId` (`KillChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Gifts
DROP TABLE IF EXISTS `Gifts`;
CREATE TABLE `Gifts` (
  `GiftId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `GiftChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `GiftDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftEngrave` varchar(64) NOT NULL DEFAULT '',
  `GiftTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `GiftProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`GiftId`),
  KEY `GiftChaId` (`GiftChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Auctions
DROP TABLE IF EXISTS `Auctions`;
CREATE TABLE `Auctions` (
  `AuctionId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AuctionExpireTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionPrice` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionBidding` tinyint(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `AuctionItmDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmEngrave` varchar(64) NOT NULL DEFAULT '',
  `AuctionItmTimer` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmCustomLook` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmCustomLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmCustomIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmCustomIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmProtected` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`AuctionId`),
  KEY `AuctionChaId` (`AuctionChaId`),
  KEY `AuctionItmId` (`AuctionItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- AuctionBids
DROP TABLE IF EXISTS `AuctionBids`;
CREATE TABLE `AuctionBids` (
  `AuctionBidId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AuctionId` int(10) unsigned NOT NULL,
  `AuctionChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `AuctionBidAmount` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`AuctionBidId`),
  KEY `AuctionChaId` (`AuctionChaId`),
  KEY `AuctionItmId` (`AuctionItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Boards (Message boards)
DROP TABLE IF EXISTS `Boards`;
CREATE TABLE `Boards` (
  `BoardId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BoardLocationId` int(10) unsigned NOT NULL DEFAULT '0',
  `BoardChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `BoardAuthor` varchar(16) NOT NULL DEFAULT '',
  `BoardTitle` varchar(64) NOT NULL DEFAULT '',
  `BoardBody` text,
  `BoardDate` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BoardId`),
  KEY `BoardLocationId` (`BoardLocationId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- BoardLocations
DROP TABLE IF EXISTS `BoardLocations`;
CREATE TABLE `BoardLocations` (
  `BlocId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BlocName` varchar(64) NOT NULL DEFAULT '',
  `BlocMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `BlocX` int(10) unsigned NOT NULL DEFAULT '0',
  `BlocY` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BlocId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- BoardNames
DROP TABLE IF EXISTS `BoardNames`;
CREATE TABLE `BoardNames` (
  `BnameId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BnameBoardId` int(10) unsigned NOT NULL DEFAULT '0',
  `BnameName` varchar(16) NOT NULL DEFAULT '',
  PRIMARY KEY (`BnameId`),
  KEY `BnameBoardId` (`BnameBoardId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- BoardTitles
DROP TABLE IF EXISTS `BoardTitles`;
CREATE TABLE `BoardTitles` (
  `BtitleId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BtitleBoardId` int(10) unsigned NOT NULL DEFAULT '0',
  `BtitleTitle` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`BtitleId`),
  KEY `BtitleBoardId` (`BtitleBoardId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Time (Server time tracking)
DROP TABLE IF EXISTS `Time`;
CREATE TABLE `Time` (
  `TimeId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `TimeYear` int(10) unsigned NOT NULL DEFAULT '0',
  `TimeMonth` int(10) unsigned NOT NULL DEFAULT '0',
  `TimeDay` int(10) unsigned NOT NULL DEFAULT '0',
  `TimeHour` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`TimeId`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

INSERT INTO `Time` VALUES (1, 1, 1, 1, 0);

-- UpTime
DROP TABLE IF EXISTS `UpTime`;
CREATE TABLE `UpTime` (
  `UpTimeId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `UpTimeStart` int(10) unsigned NOT NULL DEFAULT '0',
  `UpTimeEnd` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`UpTimeId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Maintenance
DROP TABLE IF EXISTS `Maintenance`;
CREATE TABLE `Maintenance` (
  `MaintId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MaintActive` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MaintMessage` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`MaintId`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

INSERT INTO `Maintenance` VALUES (1, 0, '');

-- BotCheckAttempts
DROP TABLE IF EXISTS `BotCheckAttempts`;
CREATE TABLE `BotCheckAttempts` (
  `BotId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `BotChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `BotAttempts` int(10) unsigned NOT NULL DEFAULT '0',
  `BotLastAttempt` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`BotId`),
  KEY `BotChaId` (`BotChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- KanPoints (Premium currency)
DROP TABLE IF EXISTS `KanPoints`;
CREATE TABLE `KanPoints` (
  `KanId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `KanAccountId` int(10) unsigned NOT NULL DEFAULT '0',
  `KanAmount` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`KanId`),
  KEY `KanAccountId` (`KanAccountId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- WisdomStar
DROP TABLE IF EXISTS `WisdomStar`;
CREATE TABLE `WisdomStar` (
  `WisId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `WisChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `WisStars` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`WisId`),
  KEY `WisChaId` (`WisChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- RankingEvents
DROP TABLE IF EXISTS `RankingEvents`;
CREATE TABLE `RankingEvents` (
  `RankEventId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `RankEventName` varchar(64) NOT NULL DEFAULT '',
  `RankEventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RankEventActive` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RankEventStart` int(10) unsigned NOT NULL DEFAULT '0',
  `RankEventEnd` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`RankEventId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- RankingScores
DROP TABLE IF EXISTS `RankingScores`;
CREATE TABLE `RankingScores` (
  `RankScoreId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `RankScoreEventId` int(10) unsigned NOT NULL DEFAULT '0',
  `RankScoreChaId` int(10) unsigned NOT NULL DEFAULT '0',
  `RankScoreValue` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`RankScoreId`),
  KEY `RankScoreEventId` (`RankScoreEventId`),
  KEY `RankScoreChaId` (`RankScoreChaId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

SELECT 'Created player/account tables' AS Status;

-- ----------------------------------------------------------------------------
-- Step 5: Create Game Data Tables
-- ----------------------------------------------------------------------------

-- Maps
DROP TABLE IF EXISTS `Maps`;
CREATE TABLE `Maps` (
  `MapId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MapName` varchar(64) NOT NULL DEFAULT '',
  `MapFile` varchar(64) NOT NULL DEFAULT '',
  `MapWidth` int(10) unsigned NOT NULL DEFAULT '0',
  `MapHeight` int(10) unsigned NOT NULL DEFAULT '0',
  `MapBgm` varchar(64) NOT NULL DEFAULT '',
  `MapFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `MapMinLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `MapMaxLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `MapNation` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`MapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- MapModifiers
DROP TABLE IF EXISTS `MapModifiers`;
CREATE TABLE `MapModifiers` (
  `ModId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ModMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `ModType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ModValue` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ModId`),
  KEY `ModMapId` (`ModMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Warps
DROP TABLE IF EXISTS `Warps`;
CREATE TABLE `Warps` (
  `WarpId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SourceMapId` int(10) NOT NULL DEFAULT '0',
  `SourceX` int(10) NOT NULL DEFAULT '0',
  `SourceY` int(10) NOT NULL DEFAULT '0',
  `DestinationMapId` int(10) NOT NULL DEFAULT '0',
  `DestinationX` int(10) NOT NULL DEFAULT '0',
  `DestinationY` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`WarpId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Paths
DROP TABLE IF EXISTS `Paths`;
CREATE TABLE `Paths` (
  `PathId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `PathMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `PathName` varchar(64) NOT NULL DEFAULT '',
  `PathX` int(10) unsigned NOT NULL DEFAULT '0',
  `PathY` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`PathId`),
  KEY `PathMapId` (`PathMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Items
DROP TABLE IF EXISTS `Items`;
CREATE TABLE `Items` (
  `ItmId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ItmName` varchar(64) NOT NULL DEFAULT '',
  `ItmDesc` varchar(255) NOT NULL DEFAULT '',
  `ItmType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ItmSubType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ItmLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmIconColor` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmLook` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmWeight` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmValue` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmDurability` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmAc` int(10) NOT NULL DEFAULT '0',
  `ItmDamage` int(10) NOT NULL DEFAULT '0',
  `ItmHit` int(10) NOT NULL DEFAULT '0',
  `ItmMight` int(10) NOT NULL DEFAULT '0',
  `ItmGrace` int(10) NOT NULL DEFAULT '0',
  `ItmWill` int(10) NOT NULL DEFAULT '0',
  `ItmVita` int(10) NOT NULL DEFAULT '0',
  `ItmMana` int(10) NOT NULL DEFAULT '0',
  `ItmPath` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ItmGender` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ItmStackable` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ItmFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `ItmScript` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`ItmId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ItemSets
DROP TABLE IF EXISTS `ItemSets`;
CREATE TABLE `ItemSets` (
  `SetId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SetName` varchar(64) NOT NULL DEFAULT '',
  `SetPieces` int(10) unsigned NOT NULL DEFAULT '0',
  `SetBonusAc` int(10) NOT NULL DEFAULT '0',
  `SetBonusDamage` int(10) NOT NULL DEFAULT '0',
  `SetBonusHit` int(10) NOT NULL DEFAULT '0',
  `SetBonusMight` int(10) NOT NULL DEFAULT '0',
  `SetBonusGrace` int(10) NOT NULL DEFAULT '0',
  `SetBonusWill` int(10) NOT NULL DEFAULT '0',
  `SetBonusVita` int(10) NOT NULL DEFAULT '0',
  `SetBonusMana` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`SetId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Recipes
DROP TABLE IF EXISTS `Recipes`;
CREATE TABLE `Recipes` (
  `RecId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `RecName` varchar(64) NOT NULL DEFAULT '',
  `RecResultItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `RecResultAmount` int(10) unsigned NOT NULL DEFAULT '1',
  `RecReq1ItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq1Amount` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq2ItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq2Amount` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq3ItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq3Amount` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq4ItmId` int(10) unsigned NOT NULL DEFAULT '0',
  `RecReq4Amount` int(10) unsigned NOT NULL DEFAULT '0',
  `RecGold` int(10) unsigned NOT NULL DEFAULT '0',
  `RecSkill` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `RecLevel` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`RecId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Spells
DROP TABLE IF EXISTS `Spells`;
CREATE TABLE `Spells` (
  `SplId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SplName` varchar(64) NOT NULL DEFAULT '',
  `SplDesc` varchar(255) NOT NULL DEFAULT '',
  `SplIcon` int(10) unsigned NOT NULL DEFAULT '0',
  `SplPath` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SplLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `SplManaCost` int(10) unsigned NOT NULL DEFAULT '0',
  `SplCastTime` int(10) unsigned NOT NULL DEFAULT '0',
  `SplCooldown` int(10) unsigned NOT NULL DEFAULT '0',
  `SplType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SplTarget` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SplRange` int(10) unsigned NOT NULL DEFAULT '0',
  `SplScript` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`SplId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

SELECT 'Created game data tables' AS Status;

-- ----------------------------------------------------------------------------
-- Step 6: Create NPC/Mob Tables
-- ----------------------------------------------------------------------------

-- Mobs
DROP TABLE IF EXISTS `Mobs`;
CREATE TABLE `Mobs` (
  `MobId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MobName` varchar(64) NOT NULL DEFAULT '',
  `MobLevel` int(10) unsigned NOT NULL DEFAULT '1',
  `MobVita` int(10) unsigned NOT NULL DEFAULT '100',
  `MobMana` int(10) unsigned NOT NULL DEFAULT '0',
  `MobExp` int(10) unsigned NOT NULL DEFAULT '0',
  `MobAc` int(10) NOT NULL DEFAULT '0',
  `MobDamageMin` int(10) unsigned NOT NULL DEFAULT '1',
  `MobDamageMax` int(10) unsigned NOT NULL DEFAULT '1',
  `MobHit` int(10) NOT NULL DEFAULT '0',
  `MobLook` int(10) unsigned NOT NULL DEFAULT '0',
  `MobLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `MobFace` int(10) unsigned NOT NULL DEFAULT '0',
  `MobAI` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MobSpeed` int(10) unsigned NOT NULL DEFAULT '500',
  `MobAtkSpeed` int(10) unsigned NOT NULL DEFAULT '1000',
  `MobRange` int(10) unsigned NOT NULL DEFAULT '1',
  `MobFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `MobScript` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`MobId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- MobEquipment
DROP TABLE IF EXISTS `MobEquipment`;
CREATE TABLE `MobEquipment` (
  `MeqId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `MeqMobId` int(10) unsigned NOT NULL DEFAULT '0',
  `MeqSlot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `MeqLook` int(10) unsigned NOT NULL DEFAULT '0',
  `MeqColor` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`MeqId`),
  KEY `MeqMobId` (`MeqMobId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- NPCs0
DROP TABLE IF EXISTS `NPCs0`;
CREATE TABLE `NPCs0` (
  `NpcId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `NpcIdent` varchar(64) NOT NULL DEFAULT '',
  `NpcName` varchar(64) NOT NULL DEFAULT '',
  `NpcMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcX` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcY` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcDirection` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `NpcLook` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcLookColor` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcFace` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `NpcScript` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`NpcId`),
  KEY `NpcIdent` (`NpcIdent`),
  KEY `NpcMapId` (`NpcMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- NPCEquipment0
DROP TABLE IF EXISTS `NPCEquipment0`;
CREATE TABLE `NPCEquipment0` (
  `NeqId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `NeqNpcId` int(10) unsigned NOT NULL DEFAULT '0',
  `NeqSlot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `NeqLook` int(10) unsigned NOT NULL DEFAULT '0',
  `NeqColor` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`NeqId`),
  KEY `NeqNpcId` (`NeqNpcId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Spawns0
DROP TABLE IF EXISTS `Spawns0`;
CREATE TABLE `Spawns0` (
  `SpawnId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SpawnMobId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnX` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnY` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnCount` int(10) unsigned NOT NULL DEFAULT '1',
  `SpawnTime` int(10) unsigned NOT NULL DEFAULT '60',
  `SpawnRange` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SpawnId`),
  KEY `SpawnMapId` (`SpawnMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Spawns1
DROP TABLE IF EXISTS `Spawns1`;
CREATE TABLE `Spawns1` (
  `SpawnId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `SpawnMobId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnX` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnY` int(10) unsigned NOT NULL DEFAULT '0',
  `SpawnCount` int(10) unsigned NOT NULL DEFAULT '1',
  `SpawnTime` int(10) unsigned NOT NULL DEFAULT '60',
  `SpawnRange` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SpawnId`),
  KEY `SpawnMapId` (`SpawnMapId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

SELECT 'Created NPC/Mob tables' AS Status;

-- ----------------------------------------------------------------------------
-- Done
-- ----------------------------------------------------------------------------
SELECT '========================================' AS '';
SELECT 'RTK Database initialization complete!' AS Status;
SELECT 'Database: RTK' AS '';
SELECT 'User: rtk' AS '';
SELECT 'Password: 50LM8U8Poq5uX2AZJVKs' AS '';
SELECT '========================================' AS '';
SELECT 'IMPORTANT: This creates empty tables.' AS '';
SELECT 'Run the full migration scripts for game data.' AS '';
SELECT '========================================' AS '';

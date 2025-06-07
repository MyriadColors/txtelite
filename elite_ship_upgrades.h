#pragma once

// Forward declarations of types from elite_ship_types.h
struct PlayerShip;
typedef struct PlayerShip PlayerShip;

// Including necessary parts directly to avoid circular dependency
#include <stdbool.h>
#include <stdint.h>

// Forward declare equipment types and functions used in this file
typedef enum EquipmentSlotType EquipmentSlotType;
typedef union EquipmentTypeSpecifics EquipmentTypeSpecifics;
#define MAX_EQUIPMENT_SLOTS 10

// Forward declare functions from elite_ship_types.h
static inline bool AddEquipment(PlayerShip *playerShip,
                         EquipmentSlotType slotType,
                         const char *equipmentName,
                         EquipmentTypeSpecifics specificType,
                         double damageOutput);

static inline bool RemoveEquipment(PlayerShip *playerShip, EquipmentSlotType slotType);

// Forward declare functions from elite_ship_inventory.h  
static inline bool RemoveEquipmentToInventory(PlayerShip *playerShip, EquipmentSlotType slotType);
static inline void MapEquipmentIndices(PlayerShip *playerShip);

#include "elite_state.h"               // For global state variables
#include "elite_equipment_constants.h" // For equipment indices

// === Constants for equipment costs and upgrade values ===
// Basic equipment costs (credits)
#define COST_PULSE_LASER 400
#define COST_BEAM_LASER 1000
#define COST_MILITARY_LASER 2500
#define COST_MINING_LASER 800
#define COST_MISSILE_HOMING 300
#define COST_MISSILE_DUMBFIRE 200
#define COST_REAR_LASER 1500
#define COST_ECM 600
#define COST_ESCAPE_POD 1000
#define COST_FUEL_SCOOPS 525
#define COST_CARGO_BAY_EXTENSION 400
#define COST_DOCKING_COMPUTER 1500
#define COST_SCANNER_UPGRADE 700

// Upgrade-related constants
#define CARGO_BAY_EXTENSION_CAPACITY 4  // 4 tonnes per cargo bay extension
#define MISSILE_PYLON_CAPACITY 4        // 4 missiles per pylon
#define SHIELD_UPGRADE_AMOUNT 25.0      // 25 additional shield points per upgrade

// Upgrade costs (credits)
#define COST_HULL_REINFORCEMENT 2500 // Cost per point of hull reinforcement
#define COST_SHIELD_ENHANCEMENT 4000 // Cost per shield upgrade level
// Update cargo bay extension cost
#undef COST_CARGO_BAY_EXTENSION
#define COST_CARGO_BAY_EXTENSION 800 // Cost per cargo bay extension
#define COST_MISSILE_PYLON 1500      // Cost per missile pylon

// Maximum upgrade levels
#define MAX_HULL_UPGRADE 50         // Maximum hull reinforcement level
#define MAX_SHIELD_UPGRADE 10       // Maximum shield enhancement level
#define MAX_CARGO_UPGRADE 5         // Maximum cargo bay extensions
#define MAX_MISSILE_PYLON_UPGRADE 3 // Maximum missile pylon upgrades

// === Upgrade Types ===
typedef enum ShipUpgradeType
{
    UPGRADE_TYPE_HULL_REINFORCEMENT,
    UPGRADE_TYPE_SHIELD_ENHANCEMENT,
    UPGRADE_TYPE_CARGO_BAY,
    UPGRADE_TYPE_MISSILE_PYLON
} ShipUpgradeType;

// === Ship-specific Upgrade Parameters ===
/**
 * Structure to hold ship-specific upgrade parameters
 */
typedef struct ShipUpgradeParameters
{
    const char *shipClass;              // Ship class name this applies to
    float hullUpgradeCostMultiplier;    // Multiplier for hull upgrade cost
    float shieldUpgradeCostMultiplier;  // Multiplier for shield upgrade cost
    float cargoUpgradeCostMultiplier;   // Multiplier for cargo upgrade cost
    float missileUpgradeCostMultiplier; // Multiplier for missile pylon upgrade cost
    int maxHullUpgrade;                 // Maximum hull upgrade level for this ship
    int maxShieldUpgrade;               // Maximum shield upgrade level for this ship
    int maxCargoUpgrade;                // Maximum cargo bay extension level for this ship
    int maxMissilePylonUpgrade;         // Maximum missile pylon upgrade level for this ship
} ShipUpgradeParameters;

// Default upgrade parameters
static const ShipUpgradeParameters DEFAULT_UPGRADE_PARAMS = {
    "Default",
    1.0f, 1.0f, 1.0f, 1.0f, // Cost multipliers (1.0 = standard)
    MAX_HULL_UPGRADE,
    MAX_SHIELD_UPGRADE,
    MAX_CARGO_UPGRADE,
    MAX_MISSILE_PYLON_UPGRADE};

// Ship-specific upgrade parameters
static const ShipUpgradeParameters SHIP_UPGRADE_PARAMS[] = {
    // Cobra Mk III - baseline ship
    {
        "Cobra Mk III",
        1.0f, 1.0f, 1.0f, 1.0f, // Standard costs
        40, 8, 5, 3             // Moderate max levels
    },
    // Viper - combat-focused ship
    {
        "Viper",
        0.9f, 0.9f, 1.3f, 0.8f, // Cheaper combat upgrades, expensive cargo
        35, 10, 3, 4            // High shield and missile capacity, limited cargo
    },
    // Asp Mk II - higher tier ship with better upgrade potential
    {
        "Asp Mk II",
        1.1f, 1.1f, 0.9f, 1.0f, // More expensive hull/shields, cheaper utilities
        50, 12, 6, 4            // Higher maximum levels overall
    }};

#define NUM_SHIP_UPGRADE_PARAMS (sizeof(SHIP_UPGRADE_PARAMS) / sizeof(ShipUpgradeParameters))

/**
 * Get upgrade parameters for a specific ship class
 *
 * @param shipClassName The name of the ship class to get parameters for
 * @return Pointer to the appropriate ShipUpgradeParameters, never NULL
 */
static inline const ShipUpgradeParameters *GetShipUpgradeParameters(const char *shipClassName)
{
    if (shipClassName == NULL)
    {
        return &DEFAULT_UPGRADE_PARAMS;
    }

    // Search for matching ship class in upgrade parameters
    for (size_t i = 0; i < NUM_SHIP_UPGRADE_PARAMS; i++)
    {
        if (strcmp(shipClassName, SHIP_UPGRADE_PARAMS[i].shipClass) == 0)
        {
            return &SHIP_UPGRADE_PARAMS[i];
        }
    }

    // Return default parameters if ship class not found
    return &DEFAULT_UPGRADE_PARAMS;
}

/**
 * Get the cost of an upgrade for a specific ship
 *
 * @param upgradeType The type of upgrade
 * @param shipParameters The ship's upgrade parameters
 * @return The cost of the upgrade in credits
 */
static inline int GetUpgradeCost(ShipUpgradeType upgradeType, const ShipUpgradeParameters *shipParameters)
{
    if (shipParameters == NULL)
    {
        return 0;
    }    switch (upgradeType)
    {
    case UPGRADE_TYPE_HULL_REINFORCEMENT:
        return (int)(COST_HULL_REINFORCEMENT * shipParameters->hullUpgradeCostMultiplier);

    case UPGRADE_TYPE_SHIELD_ENHANCEMENT:
        return (int)(COST_SHIELD_ENHANCEMENT * shipParameters->shieldUpgradeCostMultiplier);

    case UPGRADE_TYPE_CARGO_BAY:
        return (int)(COST_CARGO_BAY_EXTENSION * shipParameters->cargoUpgradeCostMultiplier);

    case UPGRADE_TYPE_MISSILE_PYLON:
        return (int)(COST_MISSILE_PYLON * shipParameters->missileUpgradeCostMultiplier);

    default:
        return 0;
    }
}

/**
 * Get the maximum upgrade level for a specific upgrade type and ship
 *
 * @param upgradeType The type of upgrade
 * @param shipParameters The ship's upgrade parameters
 * @return The maximum level for this upgrade type
 */
static inline int GetMaxUpgradeLevel(ShipUpgradeType upgradeType, const ShipUpgradeParameters *shipParameters)
{
    if (shipParameters == NULL)
    {
        return 0;
    }    switch (upgradeType)
    {
    case UPGRADE_TYPE_HULL_REINFORCEMENT:
        return shipParameters->maxHullUpgrade;

    case UPGRADE_TYPE_SHIELD_ENHANCEMENT:
        return shipParameters->maxShieldUpgrade;

    case UPGRADE_TYPE_CARGO_BAY:
        return shipParameters->maxCargoUpgrade;

    case UPGRADE_TYPE_MISSILE_PYLON:
        return shipParameters->maxMissilePylonUpgrade;

    default:
        return 0;
    }
}

/**
 * Apply a direct upgrade to a ship's core attributes, beyond just adding equipment.
 * This function is for structural/core system upgrades rather than adding separate equipment items.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param upgradeType The type of upgrade to apply
 * @param upgradeLevel The level/amount to upgrade (interpretation depends on upgradeType)
 * @param cost The cost of the upgrade in credits
 * @param externalSync If true, deduct cost from global Cash
 *
 * @return true if upgrade was successful, false otherwise
 */
static inline bool ApplyUpgrade(PlayerShip *playerShip, ShipUpgradeType upgradeType, int upgradeLevel, int cost, bool externalSync)
{
    if (playerShip == NULL || upgradeLevel <= 0)
    {
        return false;
    }

    // Check if we have enough cash to pay for the upgrade
    if (externalSync)
    {
        extern int32_t Cash;

        // Verify we can afford this upgrade
        if (cost > Cash)
        {
            printf("Insufficient credits for upgrade. Required: %d, Available: %d\n", cost, Cash);
            return false;
        }
    }

    bool upgrade_success = false;
    const char *upgrade_name = "Unknown"; // Apply upgrade based on type
    switch (upgradeType)
    {
    case UPGRADE_TYPE_HULL_REINFORCEMENT:
    {
        // Get ship-specific max upgrade level
        const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);
        int maxLevel = shipParams->maxHullUpgrade;

        // Check current hull reinforcement level
        int currentLevel = playerShip->attributes.hullStrength - playerShip->shipType->baseHullStrength;
        // Check if at max level
        if (currentLevel + upgradeLevel > maxLevel)
        {
            printf("Error: Maximum hull reinforcement level reached. Max: %d, Current: %d\n",
                   maxLevel, currentLevel);
            return false;
        }
        // Increase hull strength
        playerShip->attributes.hullStrength += upgradeLevel;
        upgrade_name = "Hull Reinforcement";
        upgrade_success = true;
        break;
    }

    case UPGRADE_TYPE_SHIELD_ENHANCEMENT:
    {
        // Get ship-specific max upgrade level
        const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);
        int maxLevel = shipParams->maxShieldUpgrade;

        // Calculate current shield level (approximate, based on default values)
        int currentLevel = (int)((playerShip->attributes.shieldStrengthFront -
                                  playerShip->shipType->baseShieldStrengthFront) /
                                 (SHIELD_UPGRADE_AMOUNT / 2.0));
        // Check if at max level
        if (currentLevel + upgradeLevel > maxLevel)
        {
            printf("Error: Maximum shield enhancement level reached. Max: %d, Current: %d\n",
                   maxLevel, currentLevel);
            return false;
        }
        // Increase shield strength
        playerShip->attributes.shieldStrengthFront += (double)upgradeLevel * (SHIELD_UPGRADE_AMOUNT / 2.0);        playerShip->attributes.shieldStrengthAft += (double)upgradeLevel * (SHIELD_UPGRADE_AMOUNT / 2.0);
        upgrade_name = "Shield Enhancement";
        upgrade_success = true;
        break;
    }

    case UPGRADE_TYPE_CARGO_BAY:
    {
        // Get ship-specific max upgrade level
        const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);
        int maxLevel = shipParams->maxCargoUpgrade;

        // Calculate current cargo level
        int currentLevel = (playerShip->attributes.cargoCapacityTons -
                            playerShip->shipType->baseCargoCapacityTons) /
                           CARGO_BAY_EXTENSION_CAPACITY;
        // Check if at max level
        if (currentLevel + upgradeLevel > maxLevel)
        {
            printf("Error: Maximum cargo bay level reached. Max: %d, Current: %d\n",
                   maxLevel, currentLevel);
            return false;
        }
        // Increase cargo capacity
        playerShip->attributes.cargoCapacityTons += upgradeLevel * CARGO_BAY_EXTENSION_CAPACITY;
        upgrade_name = "Cargo Bay Extension";
        upgrade_success = true;
        break;
    }

    case UPGRADE_TYPE_MISSILE_PYLON:
    {
        // Get ship-specific max upgrade level
        const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);
        int maxLevel = shipParams->maxMissilePylonUpgrade;

        // Calculate current pylon level
        int currentLevel = playerShip->attributes.missilePylons -
                           playerShip->shipType->initialMissilePylons;
        // Check if at max level
        if (currentLevel + upgradeLevel > maxLevel)
        {
            printf("Error: Maximum missile pylon level reached. Max: %d, Current: %d\n",
                   maxLevel, currentLevel);
            return false;
        }
        // Add missile pylons (capacity for missiles)
        playerShip->attributes.missilePylons += upgradeLevel;
        upgrade_name = "Missile Pylon";
        upgrade_success = true;
        break;
    }

    default:
        printf("Error: Unknown upgrade type.\n");
        return false;
    }

    // If upgrade succeeded and we're syncing with external state
    if (upgrade_success && externalSync)
    {
        extern int32_t Cash;

        // Deduct the cost
        Cash -= cost;

        // Special handling for cargo bay extensions if needed
        if (upgradeType == UPGRADE_TYPE_CARGO_BAY)
        {
            extern uint16_t HoldSpace;
            HoldSpace += upgradeLevel * CARGO_BAY_EXTENSION_CAPACITY;
        }
    }

    if (upgrade_success)
    {
        printf("Successfully applied %s (Level %d) for %d credits.\n", upgrade_name, upgradeLevel, cost);
    }

    return upgrade_success;
}

/**
 * Configure a preset "Combat Loadout" for a Cobra Mk III.
 * This setup prioritizes combat capabilities with military-grade weapons,
 * defensive systems, and energy management.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if loadout was successfully applied, false otherwise
 */
static inline bool ConfigureCombatLoadout(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }

    // Add military-grade weapons
    EquipmentTypeSpecifics equipType; // Use the new named union    // Forward military laser
    equipType.weaponType = WEAPON_TYPE_MILITARY_LASER;
    bool success = AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON,
        "Military Laser",
        equipType,
        10.0  // Higher damage output
    );    // Rear-mounted beam laser
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    success &= AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_AFT_WEAPON,
        "Beam Laser (Aft)",
        equipType,
        7.5   // Damage output
    );    // ECM System    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1,        "ECM System",
        equipType,
        0.0  // No damage (defensive)
    );
    // Escape Pod
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_1,
        "Escape Pod",
        equipType,
        0.0  // No damage (utility)
    );    // Apply core upgrades for combat
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 2, 0, false);  // Improved shields
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_HULL_REINFORCEMENT, 20, 0, false); // Reinforced hull
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_MISSILE_PYLON, 2, 0, false);       // Missile pylons

    // Add a full complement of missiles
    if (success)
    {
        playerShip->attributes.missilesLoadedHoming = 4;   // 4 homing missiles
        playerShip->attributes.missilesLoadedDumbfire = 4; // 4 dumbfire missiles
    }

    if (success)
    {
        printf("Combat loadout successfully configured.\n");
    }
    else
    {
        printf("Error configuring combat loadout. Some equipment may not have been installed.\n");
    }

    return success;
}

/**
 * Configure a preset "Trading Loadout" for a Cobra Mk III.
 * This setup prioritizes cargo capacity, defensive capabilities,
 * and quality-of-life improvements for traders.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if loadout was successfully applied, false otherwise
 */
static inline bool ConfigureTradingLoadout(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }

    // Add trading-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
                                  // Forward beam laser (better than pulse, less energy than military)
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    bool success = AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON,
        "Beam Laser",
        equipType,
        7.5   // Damage output
    );    // ECM System for missile defense
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1,
        "ECM System",
        equipType,
        0.0  // No damage (defensive)
    );    // Docking Computer for easier station docking
    equipType.utilityType = UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_1,
        "Docking Computer",
        equipType,
        0.0  // No damage (utility)
    );    // Cargo Bay Extension in Utility Slot 2
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_2,
        "Cargo Bay Extension",
        equipType,
        0.0  // No damage (utility)
    );    // Cargo Bay Extension in Utility Slot 3
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_3,
        "Cargo Bay Extension",
        equipType,
        0.0  // No damage (utility)
    );

    // Apply core upgrades for trading
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_CARGO_BAY, 3, 0, false);          // Maximized cargo space
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 1, 0, false); // Basic shield improvement

    if (success)
    {
        printf("Trading loadout successfully configured.\n");
        printf("Cargo capacity increased to %d tonnes.\n", playerShip->attributes.cargoCapacityTons);
    }
    else
    {
        printf("Error configuring trading loadout. Some equipment may not have been installed.\n");
    }

    return success;
}

/**
 * Configure a preset "Explorer Loadout" for a Cobra Mk III.
 * This setup prioritizes range, fuel efficiency, and self-sufficiency
 * for long journeys through space.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if loadout was successfully applied, false otherwise
 */
static inline bool ConfigureExplorerLoadout(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }

    // Add explorer-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
                                  // Forward pulse laser (standard)
    equipType.weaponType = WEAPON_TYPE_PULSE_LASER;
    bool success = AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON,
        "Pulse Laser",
        equipType,
        5.0   // Damage output
    );    // Fuel Scoops - essential for explorers
    equipType.utilityType = UTILITY_SYSTEM_TYPE_FUEL_SCOOPS;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_1,
        "Fuel Scoops",
        equipType,
        0.0  // No damage (utility)
    );       // Scanner Upgrade
    equipType.utilityType = UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_2,
        "Advanced Scanner",
        equipType,
        0.0  // No damage (utility)
    );       // Escape Pod for safety
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_3,
        "Escape Pod",
        equipType,
        0.0  // No damage (utility)
    );    // Apply core upgrades for exploration
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_HULL_REINFORCEMENT, 10, 0, false); // Some hull reinforcement

    // Fill fuel tanks to maximum
    playerShip->attributes.fuelLiters = playerShip->shipType->maxFuelLY * 100.0; // Assuming 100L = 1LY

    if (success)
    {
        printf("Explorer loadout successfully configured.\n");
        printf("Fuel scoops and advanced navigation equipment installed.\n");
    }
    else
    {
        printf("Error configuring explorer loadout. Some equipment may not have been installed.\n");
    }

    return success;
}

/**
 * Configure a preset "Mining Loadout" for a Cobra Mk III.
 * This setup prioritizes mining capabilities, cargo capacity,
 * and defensive equipment for protection in asteroid fields.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if loadout was successfully applied, false otherwise
 */
static inline bool ConfigureMiningLoadout(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }    // Add mining-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
                                      // Forward Mining Laser - essential for asteroid mining
    equipType.weaponType = WEAPON_TYPE_MINING_LASER;
    bool success = AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON,
        "Mining Laser",
        equipType,
        3.0   // Less combat damage, but effective on asteroids
    );
    // Beam Laser - for defense against pirates
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    success &= AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_AFT_WEAPON,
        "Beam Laser (Aft)",
        equipType,
        7.5   // Damage output
    );
    // ECM System for defense
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip,
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1,
        "ECM System",
        equipType,
        0.0  // No damage (defensive)
    );
    // Cargo Bay Extension in Utility Slot 1
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_1,
        "Cargo Bay Extension",
        equipType,
        0.0  // No damage (utility)
    );
    // Cargo Bay Extension in Utility Slot 2
    success &= AddEquipment(
        playerShip,
        UTILITY_SYSTEM_2,
        "Cargo Bay Extension",
        equipType,
        0.0  // No damage (utility)
    );    // Apply core upgrades for mining
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_CARGO_BAY, 2, 0, false);          // More cargo space for mined materials
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 1, 0, false); // Shield improvement for debris protection

    if (success)
    {
        printf("Mining loadout successfully configured.\n");
        printf("Mining laser and expanded cargo capacity installed.\n");
    }
    else
    {
        printf("Error configuring mining loadout. Some equipment may not have been installed.\n");
    }

    return success;
}

/**
 * Purchases equipment for the player's ship from a station.
 * This integrates with the global economy and verifies that the player has enough cash.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param equipmentName Name of the equipment to purchase
 * @param slotType The type of slot this equipment will be installed in
 * @param specificType A union containing the type-specific data
 * @param cost The cost of the equipment in credits
 * @param techLevelRequired The minimum tech level required for the equipment
 *
 * @return true if purchase successful, false otherwise
 */
static inline bool PurchaseEquipment(PlayerShip *playerShip,
                              const char *equipmentName,
                              EquipmentSlotType slotType,
                              EquipmentTypeSpecifics specificType, // Changed to named union
                              int cost,
                              int techLevelRequired,
                              double damageOutput)
{

    if (playerShip == NULL || equipmentName == NULL)
    {
        return false;
    }

    // Get external variables
    extern int32_t Cash;
    extern struct PlanSys Galaxy[];
    extern int CurrentPlanet;

    // Check if we have enough credits
    if (cost > Cash)
    {
        printf("Insufficient credits to purchase %s. Required: %d, Available: %d\n",
               equipmentName, cost, Cash);
        return false;
    }

    // Check if the current system has the required tech level
    if (Galaxy[CurrentPlanet].techLev < techLevelRequired)
    {
        printf("This equipment is not available at this technology level.\n");
        printf("Required tech level: %d, Current system tech level: %d\n",
               techLevelRequired, Galaxy[CurrentPlanet].techLev);
        return false;
    }    // Try to add the equipment
    bool equipmentAdded = AddEquipment(
        playerShip,
        slotType,
        equipmentName,
        specificType,
        damageOutput);
    if (equipmentAdded)
    {
        // Deduct cost
        Cash -= cost;
        printf("Successfully purchased %s for %d credits.\n", equipmentName, cost);

// Update equipment mapping for quick access
#include "elite_equipment_constants.h"
        MapEquipmentIndices(playerShip);

        return true;
    }
    else
    {
        printf("Failed to install %s. Make sure the slot is empty.\n", equipmentName);
        return false;
    }
}

/**
 * Displays a list of upgrades available for purchase at the shipyard
 *
 * @param playerShip Pointer to the PlayerShip structure
 */
static inline void DisplayUpgrades(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        printf("Error: Invalid ship data.\n");
        return;
    }

    // Access to player's cash from elite_state.h
    extern int32_t Cash;

    // Display upgrades header
    printf("\n=== Ship Upgrades Available ===\n");
    printf("Your current ship: %s (%s)\n", playerShip->shipName, playerShip->shipClassName);
    printf("Available credits: %.1f CR\n\n", (double)Cash / 10.0);    // Display current ship stats
    printf("Current ship specifications:\n");
    printf("  Hull Strength: %d\n", playerShip->attributes.hullStrength);
    printf("  Shield Strength: %.1f front, %.1f aft\n",
           playerShip->attributes.shieldStrengthFront,
           playerShip->attributes.shieldStrengthAft);
    printf("  Cargo Capacity: %d/%d tonnes\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);
    printf("  Missile Pylons: %d\n\n", playerShip->attributes.missilePylons);
    // Display available upgrades
    printf("Available Upgrades:\n");
    printf("%-4s %-25s %-15s %-15s %-15s\n",
           "ID", "Upgrade", "Current Level", "Cost", "Effect");
    printf("%-4s %-25s %-15s %-15s %-15s\n",
           "---", "-------", "-------------", "----", "------");

    // 1. Hull reinforcement
    // Calculate current hull upgrade level
    int currentHullLevel = playerShip->attributes.hullStrength - playerShip->shipType->baseHullStrength;
    printf("%-4d %-25s %-15d %-15d +1 Hull\n",
           1, "Hull Reinforcement", currentHullLevel,
           COST_HULL_REINFORCEMENT);

    // 2. Shield enhancement
    // Calculate current shield level (approximate, based on default values)
    int currentShieldLevel = (int)((playerShip->attributes.shieldStrengthFront -
                                    playerShip->shipType->baseShieldStrengthFront) /
                                   (SHIELD_UPGRADE_AMOUNT / 2.0));
    printf("%-4d %-25s %-15d %-15d +%.1f Shield\n",
           2, "Shield Enhancement", currentShieldLevel,
           COST_SHIELD_ENHANCEMENT, SHIELD_UPGRADE_AMOUNT);    // 3. Cargo Bay Extension
    // Calculate current cargo upgrade level
    int currentCargoLevel = (playerShip->attributes.cargoCapacityTons -
                             playerShip->shipType->baseCargoCapacityTons) /
                            CARGO_BAY_EXTENSION_CAPACITY;
    printf("%-4d %-25s %-15d %-15d +%d Cargo Space\n",
           3, "Cargo Bay Extension", currentCargoLevel,
           COST_CARGO_BAY_EXTENSION, CARGO_BAY_EXTENSION_CAPACITY);

    // 5. Missile Pylon
    // Calculate current pylon upgrade level
    int currentPylonLevel = playerShip->attributes.missilePylons -
                            playerShip->shipType->initialMissilePylons;
    printf("%-4d %-25s %-15d %-15d +1 Missile Pylon\n",
           5, "Missile Pylon", currentPylonLevel,
           COST_MISSILE_PYLON);

    // Instructions for the player
    printf("\nUse 'upgrade <ID> [quantity]' to purchase an upgrade (e.g., 'upgrade 1' or 'upgrade 2 3').\n");
    printf("Quantity is optional and defaults to 1 if not specified.\n");
}

/**
 * Display available ship upgrades at the current station
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if upgrades were displayed successfully
 */
static inline bool DisplayUpgradesShop(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        printf("Error: Invalid ship data.\n");
        return false;
    }

    // Access to player's cash
    extern int32_t Cash;
    double playerCash = (double)Cash / 10.0; // Convert to displayed value

    // Get ship-specific upgrade parameters
    const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);

    // Display upgrade shop header
    printf("\n=== Shipyard Upgrade Center ===\n");
    printf("Current ship: %s (%s)\n", playerShip->shipName, playerShip->shipClassName);
    printf("Available credits: %.1f CR\n\n", playerCash);    // Display current ship stats
    printf("Current Ship Specifications:\n");
    printf("- Hull Strength: %d\n", playerShip->attributes.hullStrength);
    printf("- Shield Strength (Front/Aft): %.1f / %.1f\n",
           playerShip->attributes.shieldStrengthFront,
           playerShip->attributes.shieldStrengthAft);
    printf("- Cargo Capacity: %d tons\n", playerShip->attributes.cargoCapacityTons);
    printf("- Missile Pylons: %d\n\n", playerShip->attributes.missilePylons);
    // Display available upgrades
    printf("Available Upgrades:\n");
    printf("%-4s %-25s %-15s %-15s %-15s\n",
           "ID", "Upgrade", "Current Level", "Cost", "Effect");
    printf("%-4s %-25s %-15s %-15s %-15s\n",
           "---", "-------", "-------------", "----", "------");

    // 1. Hull Reinforcement
    // Calculate current hull upgrade level
    int currentHullLevel = playerShip->attributes.hullStrength - playerShip->shipType->baseHullStrength;
    int maxHullLevel = shipParams->maxHullUpgrade;
    int hullCost = GetUpgradeCost(UPGRADE_TYPE_HULL_REINFORCEMENT, shipParams);
    printf("%-4d %-25s %-15d %-15.1f +1 Hull (%d max)\n",
           1, "Hull Reinforcement", currentHullLevel,
           (double)hullCost / 10.0, maxHullLevel);

    // 2. Shield Enhancement
    // Calculate current shield level (approximate, based on default values)
    int currentShieldLevel = (int)((playerShip->attributes.shieldStrengthFront -
                                    playerShip->shipType->baseShieldStrengthFront) /
                                   (SHIELD_UPGRADE_AMOUNT / 2.0));
    int maxShieldLevel = shipParams->maxShieldUpgrade;
    int shieldCost = GetUpgradeCost(UPGRADE_TYPE_SHIELD_ENHANCEMENT, shipParams);
    printf("%-4d %-25s %-15d %-15.1f +%.1f Shield (%d max)\n",
           2, "Shield Enhancement", currentShieldLevel,
           (double)shieldCost / 10.0, SHIELD_UPGRADE_AMOUNT, maxShieldLevel);    // 3. Cargo Bay Extension
    // Calculate current cargo upgrade level
    int currentCargoLevel = (playerShip->attributes.cargoCapacityTons -
                             playerShip->shipType->baseCargoCapacityTons) /
                            CARGO_BAY_EXTENSION_CAPACITY;
    int maxCargoLevel = shipParams->maxCargoUpgrade;
    int cargoCost = GetUpgradeCost(UPGRADE_TYPE_CARGO_BAY, shipParams);
    printf("%-4d %-25s %-15d %-15.1f +%d Cargo Space (%d max)\n",
           3, "Cargo Bay Extension", currentCargoLevel,
           (double)cargoCost / 10.0, CARGO_BAY_EXTENSION_CAPACITY, maxCargoLevel);

    // 4. Missile Pylon
    // Calculate current pylon upgrade level
    int currentPylonLevel = playerShip->attributes.missilePylons -
                            playerShip->shipType->initialMissilePylons;
    int maxPylonLevel = shipParams->maxMissilePylonUpgrade;
    int pylonCost = GetUpgradeCost(UPGRADE_TYPE_MISSILE_PYLON, shipParams);
    printf("%-4d %-25s %-15d %-15.1f +1 Missile Pylon (%d max)\n",
           5, "Missile Pylon", currentPylonLevel,
           (double)pylonCost / 10.0, maxPylonLevel);

    // Instructions for the player
    printf("\nUse 'upgrade <ID> [quantity]' to purchase an upgrade (e.g., 'upgrade 1' or 'upgrade 2 3').\n");
    printf("Quantity is optional and defaults to 1 if not specified.\n");
    // Show ships have different upgrade capabilities    printf("\nNote: Different ship classes have different upgrade capabilities and costs.\n");

    return true;
}

/**
 * Process a ship upgrade purchase
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param upgradeId ID of the upgrade to purchase (1-5)
 * @param quantity Quantity of the upgrade to purchase
 * @return true if purchase was successful
 */
static inline bool PurchaseUpgrade(PlayerShip *playerShip, int upgradeId, int quantity)
{
    if (playerShip == NULL || upgradeId < 1 || upgradeId > 4 || quantity <= 0)
    {
        printf("Error: Invalid upgrade parameters.\n");
        return false;
    }

    // Get ship-specific upgrade parameters
    const ShipUpgradeParameters *shipParams = GetShipUpgradeParameters(playerShip->shipClassName);

    // Map upgrade ID to upgrade type
    ShipUpgradeType upgradeType;
    const char *upgradeName = "Unknown";

    switch (upgradeId)
    {    case 1: // Hull Reinforcement
        upgradeType = UPGRADE_TYPE_HULL_REINFORCEMENT;
        upgradeName = "Hull Reinforcement";
        break;

    case 2: // Shield Enhancement
        upgradeType = UPGRADE_TYPE_SHIELD_ENHANCEMENT;
        upgradeName = "Shield Enhancement";
        break;

    case 3: // Cargo Bay Extension
        upgradeType = UPGRADE_TYPE_CARGO_BAY;
        upgradeName = "Cargo Bay Extension";
        break;

    case 4: // Missile Pylon
        upgradeType = UPGRADE_TYPE_MISSILE_PYLON;
        upgradeName = "Missile Pylon";
        break;

    default:
        printf("Error: Invalid upgrade ID.\n");
        return false;
    }

    // Get ship-specific cost per unit using the GetUpgradeCost function
    int costPerUnit = GetUpgradeCost(upgradeType, shipParams);

    // Calculate total cost
    int totalCost = costPerUnit * quantity;

    // Access to player's cash
    extern int32_t Cash;
    double playerCash = (double)Cash / 10.0; // Convert to displayed value

    // Check if player can afford the upgrade
    if (totalCost > playerCash * 10.0)
    {
        printf("Error: Insufficient credits for %s x%d.\n", upgradeName, quantity);
        printf("Required: %.1f CR, Available: %.1f CR\n",
               (double)totalCost / 10.0, playerCash);
        return false;
    }

    // Apply the upgrade
    bool success = ApplyUpgrade(playerShip, upgradeType, quantity, totalCost, true);

    if (success)
    {
        printf("Successfully purchased %s x%d for %.1f CR.\n",
               upgradeName, quantity, (double)totalCost / 10.0);
        playerCash = (double)Cash / 10.0; // Update displayed cash
        printf("Remaining credits: %.1f CR\n", playerCash);
    }

    return success;
}

/**
 * Command handler for the 'upgrade' command
 * Allows the player to view available upgrades and purchase them
 *
 * @param arguments Command arguments (upgrade ID and optional quantity)
 * @return true if command handled successfully
 */
static inline bool UpgradeCommand(const char *arguments)
{
    // Check if player is docked at a station
    extern int PlayerLocationType;

    // Need to be docked at a station (PlayerLocationType == 10 means docked)
    if (PlayerLocationType != 10)
    {
        printf("Error: You must be docked at a station to access ship upgrades.\n");
        return false;
    }

    // Get player ship
    extern PlayerShip *PlayerShipPtr; // From elite_player_state.h

    // If no arguments, just display the upgrade shop
    if (arguments == NULL || arguments[0] == '\0')
    {
        return DisplayUpgradesShop(PlayerShipPtr);
    }

    // Parse arguments for purchase
    int upgradeId = 0;
    int quantity = 1; // Default quantity

    // Try to parse as "upgrade <ID> [quantity]"
    char *endptr;
    char *args = (char*)arguments; // Cast away const for parsing
    
    // Parse upgrade ID
    upgradeId = (int)strtol(args, &endptr, 10);
    if (endptr == args || upgradeId <= 0)
    {
        printf("Error: Invalid upgrade command format.\n");
        printf("Usage: upgrade [ID] [quantity]\n");
        printf("Example: upgrade 2 3 (to buy 3 shield enhancements)\n");
        return false;
    }
    
    // Skip whitespace after upgrade ID
    while (*endptr == ' ' || *endptr == '\t')
    {
        endptr++;
    }
    
    // Parse optional quantity if present
    if (*endptr != '\0')
    {
        char *qty_endptr;
        long qty_val = strtol(endptr, &qty_endptr, 10);
        if (qty_endptr != endptr && qty_val > 0)
        {
            quantity = (int)qty_val;
        }
    }

    // Make sure quantity is positive
    if (quantity <= 0)
    {
        quantity = 1;
    }

    // Purchase the upgrade
    return PurchaseUpgrade(PlayerShipPtr, upgradeId, quantity);
}

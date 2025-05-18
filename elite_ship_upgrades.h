#pragma once

// Forward declarations of types from elite_ship_types.h
struct PlayerShip;
typedef struct PlayerShip PlayerShip;

// Including necessary parts directly to avoid circular dependency
#include <stdbool.h>
#include <stdint.h>

// Forward declare equipment types and functions used in this file
typedef enum EquipmentSlotType EquipmentSlotType;
typedef struct EquipmentTypeSpecifics EquipmentTypeSpecifics;
#define MAX_EQUIPMENT_SLOTS 10

// Function declarations from other files that we need
extern bool RemoveEquipment(PlayerShip* playerShip, EquipmentSlotType slotType);
extern bool AddEquipment(PlayerShip* playerShip, EquipmentSlotType slotType, 
                       const char* equipmentName, EquipmentTypeSpecifics specificType,
                       double energyDraw, double damageOutput);

#include "elite_state.h"      // For global state variables

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
#define COST_EXTRA_ENERGY_UNIT 1500
#define COST_ESCAPE_POD 1000
#define COST_FUEL_SCOOPS 525
#define COST_CARGO_BAY_EXTENSION 400
#define COST_DOCKING_COMPUTER 1500
#define COST_GALACTIC_HYPERSPACE 5000
#define COST_SCANNER_UPGRADE 700

// Upgrade-related constants
#define CARGO_BAY_EXTENSION_CAPACITY 4  // 4 tonnes per cargo bay extension
#define MISSILE_PYLON_CAPACITY 4        // 4 missiles per pylon
#define SHIELD_UPGRADE_AMOUNT 25.0      // 25 additional shield points per upgrade
#define EXTRA_ENERGY_UNIT_CAPACITY 50.0 // 50 additional energy bank capacity

// === Upgrade Types ===
typedef enum ShipUpgradeType {
    UPGRADE_TYPE_HULL_REINFORCEMENT,
    UPGRADE_TYPE_SHIELD_ENHANCEMENT,
    UPGRADE_TYPE_ENERGY_UNIT,
    UPGRADE_TYPE_CARGO_BAY,
    UPGRADE_TYPE_MISSILE_PYLON
} ShipUpgradeType;

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
inline bool ApplyUpgrade(PlayerShip* playerShip, ShipUpgradeType upgradeType, int upgradeLevel, int cost, bool externalSync) {
    if (playerShip == NULL || upgradeLevel <= 0) {
        return false;
    }
    
    // Check if we have enough cash to pay for the upgrade
    if (externalSync) {
        extern int32_t Cash;
        
        // Verify we can afford this upgrade
        if (cost > Cash) {
            printf("Insufficient credits for upgrade. Required: %d, Available: %d\n", cost, Cash);
            return false;
        }
    }
    
    bool upgrade_success = false;
    const char* upgrade_name = "Unknown";
    
    // Apply upgrade based on type
    switch (upgradeType) {
        case UPGRADE_TYPE_HULL_REINFORCEMENT:
            // Increase hull strength
            playerShip->attributes.hullStrength += upgradeLevel;
            upgrade_name = "Hull Reinforcement";
            upgrade_success = true;
            break;
            
        case UPGRADE_TYPE_SHIELD_ENHANCEMENT:
            // Increase shield strength
            playerShip->attributes.shieldStrengthFront += (double)upgradeLevel * (SHIELD_UPGRADE_AMOUNT / 2.0);
            playerShip->attributes.shieldStrengthAft += (double)upgradeLevel * (SHIELD_UPGRADE_AMOUNT / 2.0);
            upgrade_name = "Shield Enhancement";
            upgrade_success = true;
            break;
            
        case UPGRADE_TYPE_ENERGY_UNIT:
            // Increase energy banks capacity
            playerShip->attributes.energyBanks += (double)upgradeLevel * EXTRA_ENERGY_UNIT_CAPACITY;
            upgrade_name = "Energy Bank Expansion";
            upgrade_success = true;
            break;
            
        case UPGRADE_TYPE_CARGO_BAY:
            // Increase cargo capacity
            playerShip->attributes.cargoCapacityTons += upgradeLevel * CARGO_BAY_EXTENSION_CAPACITY;
            upgrade_name = "Cargo Bay Extension";
            upgrade_success = true;
            break;
            
        case UPGRADE_TYPE_MISSILE_PYLON:
            // Add missile pylons (capacity for missiles)
            playerShip->attributes.missilePylons += upgradeLevel;
            upgrade_name = "Missile Pylon";
            upgrade_success = true;
            break;
            
        default:
            printf("Error: Unknown upgrade type.\n");
            return false;
    }
    
    // If upgrade succeeded and we're syncing with external state
    if (upgrade_success && externalSync) {
        extern int32_t Cash;
        
        // Deduct the cost
        Cash -= cost;
        
        // Special handling for cargo bay extensions if needed
        if (upgradeType == UPGRADE_TYPE_CARGO_BAY) {
            extern uint16_t HoldSpace;
            HoldSpace += upgradeLevel * CARGO_BAY_EXTENSION_CAPACITY;
        }
    }
    
    if (upgrade_success) {
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
inline bool ConfigureCombatLoadout(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive) {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }
    
    // Add military-grade weapons
    EquipmentTypeSpecifics equipType; // Use the new named union
    
    // Forward military laser
    equipType.weaponType = WEAPON_TYPE_MILITARY_LASER;
    bool success = AddEquipment(
        playerShip, 
        "Military Laser", 
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON, 
        equipType, 
        15.0,  // Higher energy draw 
        10.0   // Higher damage output
    );
    
    // Rear-mounted beam laser
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    success &= AddEquipment(
        playerShip, 
        "Beam Laser (Aft)", 
        EQUIPMENT_SLOT_TYPE_AFT_WEAPON, 
        equipType, 
        12.0,  // Energy draw 
        7.5    // Damage output
    );
    
    // ECM System
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip, 
        "ECM System", 
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1, 
        equipType, 
        5.0,   // Energy draw 
        0.0    // No damage (defensive)
    );
    
    // Extra Energy Unit
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT;
    success &= AddEquipment(
        playerShip, 
        "Extra Energy Unit", 
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_2, 
        equipType, 
        0.0,   // No ongoing draw (it provides energy) 
        0.0    // No damage (defensive)
    );
    
    // Escape Pod
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    success &= AddEquipment(
        playerShip, 
        "Escape Pod", 
        UTILITY_SYSTEM_1, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Apply core upgrades for combat
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 2, 0, false); // Improved shields
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_HULL_REINFORCEMENT, 20, 0, false); // Reinforced hull
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_ENERGY_UNIT, 2, 0, false); // Extra energy capacity
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_MISSILE_PYLON, 2, 0, false); // Missile pylons
    
    // Add a full complement of missiles
    if (success) {
        playerShip->attributes.missilesLoadedHoming = 4;  // 4 homing missiles
        playerShip->attributes.missilesLoadedDumbfire = 4; // 4 dumbfire missiles
    }
    
    if (success) {
        printf("Combat loadout successfully configured.\n");
    } else {
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
inline bool ConfigureTradingLoadout(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive) {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }
    
    // Add trading-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
    
    // Forward beam laser (better than pulse, less energy than military)
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    bool success = AddEquipment(
        playerShip, 
        "Beam Laser", 
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON, 
        equipType, 
        12.0,  // Energy draw 
        7.5    // Damage output
    );
    
    // ECM System for missile defense
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip, 
        "ECM System", 
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1, 
        equipType, 
        5.0,   // Energy draw 
        0.0    // No damage (defensive)
    );
    
    // Docking Computer for easier station docking
    equipType.utilityType = UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER;
    success &= AddEquipment(
        playerShip, 
        "Docking Computer", 
        UTILITY_SYSTEM_1, 
        equipType, 
        2.0,   // Small energy draw 
        0.0    // No damage (utility)
    );
    
    // Cargo Bay Extension in Utility Slot 2
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    success &= AddEquipment(
        playerShip, 
        "Cargo Bay Extension", 
        UTILITY_SYSTEM_2, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Cargo Bay Extension in Utility Slot 3
    success &= AddEquipment(
        playerShip, 
        "Cargo Bay Extension", 
        UTILITY_SYSTEM_3, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Apply core upgrades for trading
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_CARGO_BAY, 3, 0, false); // Maximized cargo space
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 1, 0, false); // Basic shield improvement
    
    if (success) {
        printf("Trading loadout successfully configured.\n");
        printf("Cargo capacity increased to %d tonnes.\n", playerShip->attributes.cargoCapacityTons);
    } else {
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
inline bool ConfigureExplorerLoadout(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive) {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }
    
    // Add explorer-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
    
    // Forward pulse laser (standard)
    equipType.weaponType = WEAPON_TYPE_PULSE_LASER;
    bool success = AddEquipment(
        playerShip, 
        "Pulse Laser", 
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON, 
        equipType, 
        10.0,  // Energy draw 
        5.0    // Damage output
    );
    
    // Fuel Scoops - essential for explorers
    equipType.utilityType = UTILITY_SYSTEM_TYPE_FUEL_SCOOPS;
    success &= AddEquipment(
        playerShip, 
        "Fuel Scoops", 
        UTILITY_SYSTEM_1, 
        equipType, 
        3.0,   // Energy when scooping 
        0.0    // No damage (utility)
    );
    
    // Scanner Upgrade
    equipType.utilityType = UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE;
    success &= AddEquipment(
        playerShip, 
        "Advanced Scanner", 
        UTILITY_SYSTEM_2, 
        equipType, 
        4.0,   // Energy draw 
        0.0    // No damage (utility)
    );
    
    // Galactic Hyperspace Drive - for exceptional range
    equipType.utilityType = UTILITY_SYSTEM_TYPE_GALACTIC_HYPERSPACE_DRIVE;
    success &= AddEquipment(
        playerShip, 
        "Galactic Hyperspace", 
        UTILITY_SYSTEM_3, 
        equipType, 
        20.0,  // High energy use for jump 
        0.0    // No damage (utility)
    );
    
    // Escape Pod for safety
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    success &= AddEquipment(
        playerShip, 
        "Escape Pod", 
        UTILITY_SYSTEM_4, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Apply core upgrades for exploration
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_ENERGY_UNIT, 1, 0, false); // Extra energy for long journeys
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_HULL_REINFORCEMENT, 10, 0, false); // Some hull reinforcement
    
    // Fill fuel tanks to maximum
    playerShip->attributes.fuelLiters = COBRA_MK3_MAX_FUEL_LY * 100.0; // Assuming 100L = 1LY
    
    if (success) {
        printf("Explorer loadout successfully configured.\n");
        printf("Fuel scoops and advanced navigation equipment installed.\n");
    } else {
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
inline bool ConfigureMiningLoadout(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Clear any existing equipment by iterating through all slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive) {
            RemoveEquipment(playerShip, (EquipmentSlotType)i);
        }
    }
    
    // Add mining-focused equipment
    EquipmentTypeSpecifics equipType; // Use the new named union
    
    // Forward Mining Laser - essential for asteroid mining
    equipType.weaponType = WEAPON_TYPE_MINING_LASER;
    bool success = AddEquipment(
        playerShip, 
        "Mining Laser", 
        EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON, 
        equipType, 
        12.0,  // Energy draw 
        3.0    // Less combat damage, but effective on asteroids
    );
    
    // Beam Laser - for defense against pirates
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    success &= AddEquipment(
        playerShip, 
        "Beam Laser (Aft)", 
        EQUIPMENT_SLOT_TYPE_AFT_WEAPON, 
        equipType, 
        12.0,  // Energy draw 
        7.5    // Damage output
    );
    
    // ECM System for defense
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    success &= AddEquipment(
        playerShip, 
        "ECM System", 
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_1, 
        equipType, 
        5.0,   // Energy draw 
        0.0    // No damage (defensive)
    );
    
    // Extra Energy Unit for mining operations
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT;
    success &= AddEquipment(
        playerShip, 
        "Extra Energy Unit", 
        EQUIPMENT_SLOT_TYPE_DEFENSIVE_2, 
        equipType, 
        0.0,   // No ongoing draw (it provides energy) 
        0.0    // No damage (defensive)
    );
    
    // Cargo Bay Extension in Utility Slot 1
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    success &= AddEquipment(
        playerShip, 
        "Cargo Bay Extension", 
        UTILITY_SYSTEM_1, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Cargo Bay Extension in Utility Slot 2
    success &= AddEquipment(
        playerShip, 
        "Cargo Bay Extension", 
        UTILITY_SYSTEM_2, 
        equipType, 
        0.0,   // No energy draw 
        0.0    // No damage (utility)
    );
    
    // Apply core upgrades for mining
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_ENERGY_UNIT, 2, 0, false); // Extra energy for mining lasers
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_CARGO_BAY, 2, 0, false); // More cargo space for mined materials
    success &= ApplyUpgrade(playerShip, UPGRADE_TYPE_SHIELD_ENHANCEMENT, 1, 0, false); // Shield improvement for debris protection
    
    if (success) {
        printf("Mining loadout successfully configured.\n");
        printf("Mining laser and expanded cargo capacity installed.\n");
    } else {
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
inline bool PurchaseEquipment(PlayerShip* playerShip, 
                              const char* equipmentName, 
                              EquipmentSlotType slotType, 
                              EquipmentTypeSpecifics specificType, // Changed to named union 
                              int cost, 
                              double energyDraw, 
                              double damageOutput) {
    
    if (playerShip == NULL || equipmentName == NULL) {
        return false;
    }
    
    // Get external variables
    extern int32_t Cash;
    extern struct PlanSys Galaxy[];
    extern int CurrentPlanet;
    
    // Check if we have enough credits
    if (cost > Cash) {
        printf("Insufficient credits to purchase %s. Required: %d, Available: %d\n", 
               equipmentName, cost, Cash);
        return false;
    }
    
    // Check if the current system has the required tech level
    if (Galaxy[CurrentPlanet].techLev < techLevelRequired) {
        printf("This equipment is not available at this technology level.\n");
        printf("Required tech level: %d, Current system tech level: %d\n", 
               techLevelRequired, Galaxy[CurrentPlanet].techLev);
        return false;
    }
    
    // Try to add the equipment
    bool equipmentAdded = AddEquipment(
        playerShip, 
        equipmentName, 
        slotType, 
        specificType, 
        energyDraw, 
        damageOutput
    );
    
    if (equipmentAdded) {
        // Deduct cost
        Cash -= cost;
        printf("Successfully purchased %s for %d credits.\n", equipmentName, cost);
        return true;
    } else {
        printf("Failed to install %s. Make sure the slot is empty.\n", equipmentName);
        return false;
    }
}

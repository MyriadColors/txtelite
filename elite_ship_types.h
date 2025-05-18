#pragma once

// include elite_ship_upgrades.h at the end for upgrade functionality
// #include "elite_ship_upgrades.h" // Moved to end of file to avoid circular dependencies
#include <stddef.h>  // For NULL
#include <stdint.h>  // For uint16_t, int32_t
#include <stdbool.h> // For bool
#include <string.h>  // For string functions
#include <stdio.h>   // For printf

// TODO: Keep this file in sync with elite_ship_equipment.h and elite_ship_cargo.h

// --- Constants ---
// Cobra Mk III Base Values
const int COBRA_MK3_BASE_HULL_STRENGTH = 100;       // Example value
const double COBRA_MK3_BASE_ENERGY_BANKS = 100.0;   // Example value
const double COBRA_MK3_BASE_SHIELD_STRENGTH = 50.0; // Example value, assuming combined for now
const double COBRA_MK3_MAX_FUEL_LY = 7.0;
const int COBRA_MK3_BASE_CARGO_CAPACITY_TONS = 20;
const int COBRA_MK3_INITIAL_MISSILE_PYLONS = 0;

// --- Constants for Equipment Types ---
#define MAX_SHIP_NAME_LENGTH 64
#define MAX_EQUIPMENT_SLOTS 10 // Max possible equipment slots
#define MAX_CARGO_SLOTS 50     // Max types of cargo a ship can hold

// --- Enumerations ---

typedef enum WeaponType
{
    WEAPON_TYPE_NONE,
    WEAPON_TYPE_PULSE_LASER,
    WEAPON_TYPE_BEAM_LASER,
    WEAPON_TYPE_MILITARY_LASER,
    WEAPON_TYPE_MINING_LASER,
    WEAPON_TYPE_MISSILE_HOMING,
    WEAPON_TYPE_MISSILE_DUMBFIRE,
    WEAPON_TYPE_REAR_LASER // Generic rear laser, specific type can be an attribute
} WeaponType;

typedef enum DefensiveSystemType
{
    DEFENSIVE_SYSTEM_TYPE_NONE,
    DEFENSIVE_SYSTEM_TYPE_ECM,              // Electronic Counter-Measures
    DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT // EEU
} DefensiveSystemType;

typedef enum UtilitySystemType
{
    UTILITY_SYSTEM_TYPE_NONE,
    UTILITY_SYSTEM_TYPE_ESCAPE_POD,
    UTILITY_SYSTEM_TYPE_FUEL_SCOOPS,
    UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION, // Represents the upgrade itself
    UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER,
    UTILITY_SYSTEM_TYPE_GALACTIC_HYPERSPACE_DRIVE,
    UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE
} UtilitySystemType;

// --- Named Union for Equipment Specifics ---
typedef union EquipmentTypeSpecifics
{
    WeaponType weaponType;
    DefensiveSystemType defensiveType;
    UtilitySystemType utilityType;
} EquipmentTypeSpecifics;

typedef enum EquipmentSlotType
{
    EQUIPMENT_SLOT_TYPE_NONE,
    EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON,
    EQUIPMENT_SLOT_TYPE_AFT_WEAPON,
    EQUIPMENT_SLOT_TYPE_DEFENSIVE_1,
    EQUIPMENT_SLOT_TYPE_DEFENSIVE_2,
    UTILITY_SYSTEM_1,
    UTILITY_SYSTEM_2,
    UTILITY_SYSTEM_3,
    UTILITY_SYSTEM_4
} EquipmentSlotType;

// --- Structures ---

typedef struct ShipCoreAttributes
{
    int hullStrength;
    double energyBanks;
    double maxEnergyBanks;
    double shieldStrengthFront;
    double shieldStrengthAft;
    double fuelLiters; // Internal representation, can be converted to LY
    int cargoCapacityTons;
    int currentCargoTons; // Actual used cargo space
    int missilePylons;
    int missilesLoadedHoming;
    int missilesLoadedDumbfire;
    // Future: speed, maneuverability
} ShipCoreAttributes;

typedef struct ShipEquipmentItem
{
    char name[MAX_SHIP_NAME_LENGTH];
    EquipmentSlotType slotType;          // What kind of slot this is (e.g. Forward Weapon)
    int isActive;                        // 0 for empty/damaged, 1 for active    // Union for specific equipment types
    EquipmentTypeSpecifics typeSpecific; // Use the new named union

    // Common attributes
    double energyDraw;   // Energy to use/activate
    double damageOutput; // For weapons
    // Add other common attributes: range, effect_duration, etc.
} ShipEquipmentItem;

typedef struct CargoItem
{
    char name[MAX_SHIP_NAME_LENGTH]; // Name of the commodity
    int quantity;                    // Number of units
    int purchasePrice;               // Price per unit when bought (for player reference)
    // Future: legality, volatility
} CargoItem;

// Forward declaration for MAX_EQUIPMENT_INVENTORY
#ifndef MAX_EQUIPMENT_INVENTORY
#define MAX_EQUIPMENT_INVENTORY 30
#endif

typedef struct PlayerShip
{
    char shipName[MAX_SHIP_NAME_LENGTH];
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // e.g., "Cobra Mk III"
    ShipCoreAttributes attributes;
    ShipEquipmentItem equipment[MAX_EQUIPMENT_SLOTS];      // Currently equipped items
    ShipEquipmentItem equipmentInventory[MAX_EQUIPMENT_INVENTORY]; // Inventory of stored equipment
    CargoItem cargo[MAX_CARGO_SLOTS];
    // Removed: int credits; Player's money is managed globally (e.g., as 'Cash')
} PlayerShip;

// --- Function Prototypes (Implementations will be in this header as per standard) ---

// Initializes a PlayerShip to Cobra Mk III default specifications.
inline void InitializeCobraMkIII(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    strncpy(playerShip->shipName, "My Cobra", MAX_SHIP_NAME_LENGTH - 1);
    playerShip->shipName[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    strncpy(playerShip->shipClassName, "Cobra Mk III", MAX_SHIP_NAME_LENGTH - 1);    playerShip->shipClassName[MAX_SHIP_NAME_LENGTH - 1] = '\0';

    playerShip->attributes.hullStrength = COBRA_MK3_BASE_HULL_STRENGTH;
    playerShip->attributes.energyBanks = COBRA_MK3_BASE_ENERGY_BANKS;
    playerShip->attributes.maxEnergyBanks = COBRA_MK3_BASE_ENERGY_BANKS;
    // Assuming shields are combined for now, split later if needed by design
    playerShip->attributes.shieldStrengthFront = COBRA_MK3_BASE_SHIELD_STRENGTH;
    playerShip->attributes.shieldStrengthAft = COBRA_MK3_BASE_SHIELD_STRENGTH;
    // Fuel in Liters, assuming 1 LY = 100 Liters for example.
    // This needs a proper conversion factor based on game balance.
    // For now, let's say max fuel is 700.0 liters if 1LY = 100L
    playerShip->attributes.fuelLiters = COBRA_MK3_MAX_FUEL_LY * 100.0;
    playerShip->attributes.cargoCapacityTons = COBRA_MK3_BASE_CARGO_CAPACITY_TONS;
    playerShip->attributes.currentCargoTons = 0;
    playerShip->attributes.missilePylons = COBRA_MK3_INITIAL_MISSILE_PYLONS;
    playerShip->attributes.missilesLoadedHoming = 0;
    playerShip->attributes.missilesLoadedDumbfire = 0;    // Initialize equipment slots to empty
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        playerShip->equipment[i].isActive = 0;
        playerShip->equipment[i].slotType = EQUIPMENT_SLOT_TYPE_NONE;
        playerShip->equipment[i].energyDraw = 0.0;
    }
    
    // Initialize inventory slots to empty
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i)
    {
        playerShip->equipmentInventory[i].isActive = 0;
        strcpy(playerShip->equipmentInventory[i].name, "Empty");
        playerShip->equipmentInventory[i].slotType = EQUIPMENT_SLOT_TYPE_NONE;
        playerShip->equipmentInventory[i].energyDraw = 0.0;
        playerShip->equipment[i].damageOutput = 0.0;
        memset(playerShip->equipment[i].name, 0, MAX_SHIP_NAME_LENGTH);
        strncpy(playerShip->equipment[i].name, "Empty", MAX_SHIP_NAME_LENGTH - 1);
        playerShip->equipment[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
        // Initialize other fields if necessary, like setting slotType to a default/none
        // For now, slotType is only actively set when equipment is added.
    }

    // Initialize cargo slots to empty
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        playerShip->cargo[i].quantity = 0;
        strncpy(playerShip->cargo[i].name, "Empty", MAX_SHIP_NAME_LENGTH - 1);
        playerShip->cargo[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    }

    // Removed setting of hasPulseLaser, hasBasicShields, etc. flags.
    // These are now determined dynamically or are inherent to the ship class.

    // Equip standard Pulse Laser for Cobra Mk III
    // This directly manipulates the equipment array as before.
    // A dedicated AddEquipmentToShip function will be used later for more complex scenarios.
    if (MAX_EQUIPMENT_SLOTS > 0)
    { // Cobra Mk III comes with a pulse laser
        strncpy(playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name, "Pulse Laser", MAX_SHIP_NAME_LENGTH - 1);
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].typeSpecific.weaponType = WEAPON_TYPE_PULSE_LASER;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].isActive = 1;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].energyDraw = 10.0;  // Example
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].damageOutput = 5.0; // Example
    }
}

// Displays the current status of the player's ship.
inline void DisplayShipStatus(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        printf("Error: Ship data is NULL.\n");
        return;
    }

    printf("\n--- %s (%s) Status ---\n", playerShip->shipName, playerShip->shipClassName);
    printf("Hull Strength: %d / %d\n", playerShip->attributes.hullStrength, COBRA_MK3_BASE_HULL_STRENGTH); // Assuming base is max for now
    printf("Energy Banks: %.2f / %.2f\n", playerShip->attributes.energyBanks, COBRA_MK3_BASE_ENERGY_BANKS);
    printf("Shields (F/A): %.2f / %.2f\n", playerShip->attributes.shieldStrengthFront, playerShip->attributes.shieldStrengthAft);
    // Convert Liters to LY for display, assuming 1 LY = 100 Liters (example factor)
    printf("Fuel: %.2f LY (%.0f Liters)\n", playerShip->attributes.fuelLiters / 100.0, playerShip->attributes.fuelLiters);
    printf("Cargo: %dT / %dT\n", playerShip->attributes.currentCargoTons, playerShip->attributes.cargoCapacityTons);
    printf("Missile Pylons: %d (Homing: %d, Dumbfire: %d)\n",
           playerShip->attributes.missilePylons,
           playerShip->attributes.missilesLoadedHoming,
           playerShip->attributes.missilesLoadedDumbfire);
    // Removed: printf("Credits: %dcr\n", playerShip->credits);    printf("\n--- Equipment ---\n");
    int hasEquipment = 0;
      for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive && 
            strlen(playerShip->equipment[i].name) > 0 &&
            strcmp(playerShip->equipment[i].name, "Empty") != 0)
        {
            hasEquipment = 1;
            printf("- %s", playerShip->equipment[i].name);
            
            // Only print slot info if it's useful
            if (playerShip->equipment[i].slotType != EQUIPMENT_SLOT_TYPE_NONE) {
                printf(" (Slot: %d", playerShip->equipment[i].slotType);
                
                // Print the type info based on slot type
                if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
                    playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
                    printf(", Type: Weapon - %d", playerShip->equipment[i].typeSpecific.weaponType);
                }
                else if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                         playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
                    printf(", Type: Defensive - %d", playerShip->equipment[i].typeSpecific.defensiveType);
                }
                else if (playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                         playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4) {
                    printf(", Type: Utility - %d", playerShip->equipment[i].typeSpecific.utilityType);
                }
                printf(")");
            }
            printf("\n");
        }
    }
    if (!hasEquipment)
    {
        printf("No active equipment.\n");
    }

    // Display standard equipment flags for clarity based on design doc
    // This section is now dynamic based on equipment array and ship class
    printf("\n--- Key Systems & Upgrades ---\n");

    bool isCobraMkIII = (strcmp(playerShip->shipClassName, "Cobra Mk III") == 0);

    // Standard Inherent features for Cobra Mk III
    if (isCobraMkIII)
    {
        printf("- Basic Shields System\n"); // Cobra Mk III always has shields
        printf("- Standard Hyperspace Drive (%.1f LY Max)\n", COBRA_MK3_MAX_FUEL_LY);
        // Standard Cargo Bay is reflected in attributes.cargoCapacityTons
        printf("- Standard Cargo Bay (%dT)\n", COBRA_MK3_BASE_CARGO_CAPACITY_TONS);
    } // Check for specific equipment types in their typical slots or any slot if generic
    bool ecmFound = false;
    bool escapePodFound = false;
    bool fuelScoopsFound = false;
    bool dockingComputerFound = false;
    bool galacticHyperspaceFound = false;
    bool scannerUpgradeFound = false;
    bool rearLaserFound = false;
    bool forwardPulseLaserFound = false;
    bool energyEnhancementFound = false;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive)
        {
            // Forward Pulse Laser (Standard for Cobra, but check if equipped)
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON &&
                playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_PULSE_LASER)
            {
                if (!forwardPulseLaserFound)
                { // Check if it's the standard one
                    if (isCobraMkIII)
                        printf("- Standard Forward Pulse Laser\n");
                    else
                        printf("- Forward Pulse Laser\n");
                    forwardPulseLaserFound = true;
                }
            } // Rear-mounted Laser
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON &&
                ((playerShip->equipment[i].typeSpecific.weaponType >= WEAPON_TYPE_PULSE_LASER &&
                     playerShip->equipment[i].typeSpecific.weaponType <= WEAPON_TYPE_MINING_LASER) ||
                 playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_REAR_LASER))
            {
                if (!rearLaserFound)
                {
                    printf("- Rear-mounted Laser\n");
                    rearLaserFound = true;
                }
            }

            // Defensive Systems
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
            {
                if (playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM && !ecmFound)
                {
                    printf("- ECM Unit\n");
                    ecmFound = true;
                }
                else if (playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT && !energyEnhancementFound)
                {
                    printf("- Energy Enhancement Unit\n");
                    energyEnhancementFound = true;
                }
            }

            // Utility Systems
            if (playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 && // Assumes utility slots are contiguous in enum
                playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4)
            {
                switch (playerShip->equipment[i].typeSpecific.utilityType)
                {
                case UTILITY_SYSTEM_TYPE_ESCAPE_POD:
                    if (!escapePodFound)
                    {
                        printf("- Escape Pod\n");
                        escapePodFound = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_FUEL_SCOOPS:
                    if (!fuelScoopsFound)
                    {
                        printf("- Fuel Scoops\n");
                        fuelScoopsFound = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER:
                    if (!dockingComputerFound)
                    {
                        printf("- Docking Computer\n");
                        dockingComputerFound = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_GALACTIC_HYPERSPACE_DRIVE:
                    if (!galacticHyperspaceFound)
                    {
                        printf("- Galactic Hyperspace Drive\n");
                        galacticHyperspaceFound = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
                    if (!scannerUpgradeFound)
                    {
                        printf("- Scanner Upgrade\n");
                        scannerUpgradeFound = true;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    // If standard pulse laser for Cobra wasn't found (e.g. replaced), but it's a Cobra, mention it's usually standard.
    // This might be too complex; the equipment list itself is the source of truth.
    // For now, if it's equipped, it's listed. If not, it's not.

    printf("\n--- Cargo Hold (%dT used / %dT capacity) ---\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);
    int hasCargo = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            hasCargo = 1;
            printf("- %s: %d units (Bought at: %dcr each)\n",
                   playerShip->cargo[i].name,
                   playerShip->cargo[i].quantity,
                   playerShip->cargo[i].purchasePrice);
        }
    }    if (!hasCargo)
    {
        printf("Cargo hold is empty.\n");
    }
    printf("---------------------------\n");
    
    // Add a note about the inventory system
    printf("\nEquipment inventory commands: 'inv', 'store <slot>', 'use <inv_idx> <slot>'\n");
}

// Include cargo management system
// #include "elite_ship_cargo.h"

// Include ship upgrade functionality
#include "elite_ship_upgrades.h"

/**
 * Checks if the ship has fuel scoops installed
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if the ship has fuel scoops, false otherwise
 */
inline bool HasFuelScoops(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }
    
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_FUEL_SCOOPS)
        {
            return true;
        }
    }
    
    return false;
}

/**
 * Refuels the player's ship, either by paying cash at a station or using fuel scoops.
 * This function handles synchronization with global game state (Cash, Fuel).
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param fuelAmountLY Amount of fuel requested in light-years (as float, e.g. 3.5 LY)
 * @param useFuelScoops If true, attempt to use ship's fuel scoops instead of paying
 * @param externalSync If true, synchronize with the global state values for
 *                     Cash and Fuel in elite_state.h
 *
 * @return The actual amount of fuel added to the ship in light-years
 */
inline float RefuelShip(PlayerShip *playerShip, float fuelAmountLY, bool useFuelScoops, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0.0f;
    }

    // Check if we're already at maximum fuel
    const float maxFuelLY = COBRA_MK3_MAX_FUEL_LY;                    // This is ship-specific (7.0 LY for Cobra Mk III)
    float currentFuelLY = playerShip->attributes.fuelLiters / 100.0f; // Convert liters to LY assuming 100L = 1LY

    if (currentFuelLY >= maxFuelLY)
    {
        printf("Fuel tanks already full (%.1f LY).\n", maxFuelLY);
        return 0.0f;
    }

    // Calculate how much more fuel can fit in the tank
    float availableSpace = maxFuelLY - currentFuelLY;

    // Limit requested amount to available space
    float effectiveRequestLY = (fuelAmountLY > availableSpace) ? availableSpace : fuelAmountLY;    // Handle fuel scooping if requested
    if (useFuelScoops)
    {
        // Check if the ship has fuel scoops installed
        if (!HasFuelScoops(playerShip))
        {
            printf("Error: Your ship is not equipped with fuel scoops.\n");
            return 0.0f;
        }

        // Fuel scooping logic would go here
        // For now, just grant the fuel without cost, but with some time delay in future
        // Note: This could be expanded to include risk/damage when close to a star

        // Add the fuel to the ship
        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f); // Convert LY to liters
        printf("Successfully scooped %.1f LY of fuel from the star.\n", effectiveRequestLY);

        // Sync with global state if requested
        if (externalSync)
        {
            // Declare external variables
            extern uint16_t Fuel;
            extern int MaxFuel;

            // Convert to the units used in elite_state.h (tenth of LY)
            uint16_t fuelToAdd = (uint16_t)(effectiveRequestLY * 10.0f);

            // Make sure not to exceed MaxFuel
            if (Fuel + fuelToAdd > (uint16_t)MaxFuel)
            {
                Fuel = (uint16_t)MaxFuel;
            }
            else
            {
                Fuel += fuelToAdd;
            }
        }

        return effectiveRequestLY;
    }
    // Standard refueling at a station (costs money)
    else
    {
        // Declare external variables
        extern int FuelCost;
        extern int32_t Cash;

        // Convert to tenths of LY for cost calculation
        uint16_t fuelUnits = (uint16_t)(effectiveRequestLY * 10.0f);
        int totalCost = fuelUnits * FuelCost;

        // Check if we can afford it
        if (externalSync && totalCost > Cash)
        {
            // Calculate how much we can afford
            uint16_t affordableUnits = (uint16_t)(Cash / FuelCost);
            fuelUnits = affordableUnits;
            totalCost = fuelUnits * FuelCost;
            effectiveRequestLY = (float)affordableUnits / 10.0f;

            if (fuelUnits == 0)
            {
                printf("Insufficient credits to purchase fuel.\n");
                return 0.0f;
            }
        }

        // Deduct the cost if we're syncing with external state
        if (externalSync)
        {
            Cash -= totalCost;
        }

        // Add the fuel to the ship
        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f); // Convert LY to liters

        // Sync with global state if requested
        if (externalSync)
        {
            extern uint16_t Fuel;
            extern int MaxFuel;

            // Make sure not to exceed MaxFuel
            if (Fuel + fuelUnits > (uint16_t)MaxFuel)
            {
                Fuel = (uint16_t)MaxFuel;
            }
            else
            {
                Fuel += fuelUnits;
            }
        }

        printf("Purchased %.1f LY of fuel for %d credits.\n", effectiveRequestLY, totalCost);
        return effectiveRequestLY;
    }
}

/**
 * Activates ECM to destroy incoming enemy missiles.
 * Energy is required to operate the ECM.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if ECM was successfully activated, false otherwise
 */
inline bool ActivateECM(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has ECM
    bool hasECM = false;
    double ecmEnergyCost = 0.0;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
             playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM)
        {
            hasECM = true;
            ecmEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }

    if (!hasECM)
    {
        printf("Error: Your ship is not equipped with ECM System.\n");
        return false;
    }

    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < ecmEnergyCost)
    {
        printf("Error: Insufficient energy to activate ECM System.\n");
        printf("Required: %.1f, Available: %.1f\n", ecmEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }

    // Consume energy
    playerShip->attributes.energyBanks -= ecmEnergyCost;

    printf("ECM System activated! All incoming missiles have been destroyed.\n");
    return true;
}

/**
 * Activates the docking computer to automatically dock with a station.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param distance The distance to the station (used to determine docking time)
 * @return true if docking computer was activated successfully, false otherwise
 */
inline bool ActivateDockingComputer(PlayerShip *playerShip, double distance)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has docking computer
    bool hasDockingComputer = false;
    double dockingComputerEnergyCost = 0.0;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER)
        {
            hasDockingComputer = true;
            dockingComputerEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }

    if (!hasDockingComputer)
    {
        printf("Error: Your ship is not equipped with a Docking Computer.\n");
        return false;
    }

    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < dockingComputerEnergyCost)
    {
        printf("Error: Insufficient energy to activate Docking Computer.\n");
        printf("Required: %.1f, Available: %.1f\n", dockingComputerEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }

    // Consume energy
    playerShip->attributes.energyBanks -= dockingComputerEnergyCost;

    // Calculate docking time based on distance
    // This is a placeholder - actual docking procedure would be implemented elsewhere
    int dockingTimeSeconds = (int)(distance * 5.0); // 5 seconds per AU for example

    printf("Docking Computer activated. Auto-docking sequence initiated.\n");
    printf("Estimated time to complete docking: %d seconds.\n", dockingTimeSeconds);

    // Here we'd normally advance the game time by dockingTimeSeconds
    // and trigger the actual docking process

    return true;
}

/**
 * Uses the ship's scanner to get enhanced information about nearby objects.
 * The quality and range of information depends on whether a scanner upgrade is installed.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if scan was successful, false otherwise
 */
inline bool UseScanner(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has advanced scanner
    bool hasUpgradedScanner = false;
    double scannerEnergyCost = 2.0; // Basic scanner energy cost

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE)
        {
            hasUpgradedScanner = true;
            scannerEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }

    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < scannerEnergyCost)
    {
        printf("Error: Insufficient energy to power scanner.\n");
        printf("Required: %.1f, Available: %.1f\n", scannerEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }

    // Consume energy
    playerShip->attributes.energyBanks -= scannerEnergyCost;

    // Perform scan
    if (hasUpgradedScanner)
    {
        printf("Advanced scanner activated. Extended range and detailed scan initiated.\n");
        // Advanced scanner would provide more detailed information
        // This would normally integrate with the navigation system
    }
    else
    {
        printf("Basic scanner activated. Standard scan initiated.\n");
        // Basic scanner would provide standard information
    }

    return true;
}

/**
 * Attempts to deploy the escape pod if the ship is critically damaged.
 * If successful, the player escapes but loses the ship and cargo.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param criticalDamage Whether the ship has taken critical damage
 * @return true if escape pod was successfully deployed, false otherwise
 */
inline bool DeployEscapePod(PlayerShip *playerShip, bool criticalDamage)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has escape pod
    bool hasEscapePod = false;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_ESCAPE_POD)
        {
            hasEscapePod = true;
            break;
        }
    }

    if (!hasEscapePod)
    {
        printf("Error: Your ship is not equipped with an Escape Pod.\n");
        return false;
    }

    // Only allow escape pod use if ship is critically damaged or override for testing
    if (!criticalDamage)
    {
        printf("Escape pod can only be deployed in case of critical ship damage.\n");
        return false;
    }

    printf("EMERGENCY: Escape pod deployed! You have been safely ejected from your ship.\n");
    printf("Your ship and cargo have been lost, but you have survived.\n");

    // This would normally trigger game logic to handle the aftermath
    // such as losing the ship and cargo, but preserving the player's life and credits

    return true;
}

/**
 * Attempts to use the Galactic Hyperspace Drive for a special long-range jump.
 * This advanced drive allows jumps to distant systems beyond normal hyperspace range.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param targetSystemName Name of the target system
 * @return true if galactic jump was successful, false otherwise
 */
inline bool UseGalacticHyperspace(PlayerShip *playerShip, const char *targetSystemName)
{
    if (playerShip == NULL || targetSystemName == NULL)
    {
        return false;
    }

    // Check if ship has galactic hyperspace drive
    bool hasGalacticDrive = false;
    double galacticDriveEnergyCost = 0.0;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_GALACTIC_HYPERSPACE_DRIVE)
        {
            hasGalacticDrive = true;
            galacticDriveEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }

    if (!hasGalacticDrive)
    {
        printf("Error: Your ship is not equipped with a Galactic Hyperspace Drive.\n");
        return false;
    }

    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < galacticDriveEnergyCost)
    {
        printf("Error: Insufficient energy to power the Galactic Hyperspace Drive.\n");
        printf("Required: %.1f, Available: %.1f\n", galacticDriveEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }

    // Consume energy
    playerShip->attributes.energyBanks -= galacticDriveEnergyCost;

    // This would normally trigger the actual galaxy jump logic
    printf("Galactic Hyperspace Drive activated!\n");
    printf("Jumping to %s system...\n", targetSystemName);

    return true;
}

/**
 * Gets the damage output of a specific weapon.
 * Used in combat calculations to determine damage dealt to targets.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The weapon slot to check (forward or aft)
 * @return The damage output value, or 0.0 if no weapon is installed
 */
inline double GetWeaponDamage(const PlayerShip *playerShip, EquipmentSlotType slotType)
{
    if (playerShip == NULL ||
        (slotType != EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON && slotType != EQUIPMENT_SLOT_TYPE_AFT_WEAPON))
    {
        return 0.0;
    }

    // Check if the weapon slot has an active weapon
    if (playerShip->equipment[slotType].isActive)
    {
        return playerShip->equipment[slotType].damageOutput;
    }

    return 0.0;
}

/**
 * Gets the energy draw of a specific equipment item.
 * Used to calculate energy consumption during various operations.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The equipment slot to check
 * @return The energy draw value, or 0.0 if no equipment is installed
 */
inline double GetEquipmentEnergyDraw(const PlayerShip *playerShip, EquipmentSlotType slotType)
{
    if (playerShip == NULL || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        return 0.0;
    }

    // Check if the slot has active equipment
    if (playerShip->equipment[slotType].isActive)
    {
        return playerShip->equipment[slotType].energyDraw;
    }

    return 0.0;
}

// Function to check if a specific type of equipment is installed
/**
 * Checks if the ship has a specific type of equipment installed.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The type of slot to check (e.g., EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON)
 * @param specificType The specific type to check (EquipmentTypeSpecifics union)
 * @return true if the equipment is installed, false otherwise
 */
inline bool HasEquipment(const PlayerShip *playerShip, EquipmentSlotType slotType, EquipmentTypeSpecifics specificType)
{
    if (playerShip == NULL)
    {
        return false;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (!playerShip->equipment[i].isActive)
        {
            continue;
        }

        // Match slot type
        if (playerShip->equipment[i].slotType == slotType)
        {
            // Match specific type based on slot type
            switch (slotType)
            {
            case EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON:
            case EQUIPMENT_SLOT_TYPE_AFT_WEAPON:
                if (playerShip->equipment[i].typeSpecific.weaponType == specificType.weaponType)
                {
                    return true;
                }
                break;

            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_1:
            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_2:
                if (playerShip->equipment[i].typeSpecific.defensiveType == specificType.defensiveType)
                {
                    return true;
                }
                break;

            case UTILITY_SYSTEM_1:
            case UTILITY_SYSTEM_2:
            case UTILITY_SYSTEM_3:
            case UTILITY_SYSTEM_4:
                if (playerShip->equipment[i].typeSpecific.utilityType == specificType.utilityType)
                {
                    return true;
                }
                break;

            default:
                break;
            }
        }
    }

    return false;
}

/**
 * Repairs the hull of the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param repairAmount Amount of hull strength to repair
 * @param costPerPoint Cost in credits per point of hull strength (if using externalSync)
 * @param externalSync If true, deduct cost from global Cash
 *
 * @return The actual amount of hull strength repaired
 */
inline int RepairHull(PlayerShip *playerShip, int repairAmount, int costPerPoint, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    // Check if hull is already at maximum
    if (playerShip->attributes.hullStrength >= COBRA_MK3_BASE_HULL_STRENGTH)
    {
        printf("Hull already at maximum strength.\n");
        return 0;
    }

    // Calculate how much more hull strength can be repaired
    int maxRepair = COBRA_MK3_BASE_HULL_STRENGTH - playerShip->attributes.hullStrength;

    // Limit repair amount
    int effectiveRepair = (repairAmount > maxRepair) ? maxRepair : repairAmount;

    // Calculate total cost
    int totalCost = effectiveRepair * costPerPoint;

    // Check if we can afford it and deduct cost if using external sync
    if (externalSync)
    {
        extern int32_t Cash;

        if (totalCost > Cash)
        {
            // Calculate how much repair we can afford
            effectiveRepair = (int)(Cash / costPerPoint);
            totalCost = effectiveRepair * costPerPoint;

            if (effectiveRepair == 0)
            {
                printf("Insufficient credits for hull repairs.\n");
                return 0;
            }
        }

        // Deduct cost
        Cash -= totalCost;
    }

    // Apply the repair
    playerShip->attributes.hullStrength += effectiveRepair;

    printf("Repaired %d hull strength points for %d credits.\n", effectiveRepair, totalCost);
    return effectiveRepair;
}

/**
 * Recharges the ship's energy banks and shields.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param rechargeAmount Amount of energy to recharge (if 0, fully recharge)
 * @param rechargeShields Whether to recharge shields as well
 * @param costPerPoint Cost in credits per point of energy (if using externalSync)
 * @param externalSync If true, deduct cost from global Cash
 *
 * @return The actual amount of energy recharged
 */
inline double RechargeEnergy(PlayerShip *playerShip, double rechargeAmount, bool rechargeShields, double costPerPoint, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0.0;
    }

    // Check current energy levels
    double energyNeeded = COBRA_MK3_BASE_ENERGY_BANKS - playerShip->attributes.energyBanks;
    double shieldFrontNeeded = 0.0;
    double shieldAftNeeded = 0.0;

    if (rechargeShields)
    {
        shieldFrontNeeded = COBRA_MK3_BASE_SHIELD_STRENGTH - playerShip->attributes.shieldStrengthFront;
        shieldAftNeeded = COBRA_MK3_BASE_SHIELD_STRENGTH - playerShip->attributes.shieldStrengthAft;
    }

    // Calculate total energy needed
    double totalNeeded = energyNeeded + shieldFrontNeeded + shieldAftNeeded;

    // If nothing needs recharging
    if (totalNeeded <= 0.0)
    {
        printf("Energy banks and shields are already fully charged.\n");
        return 0.0;
    }

    // Determine amount to recharge
    double effectiveRecharge = (rechargeAmount <= 0.0 || rechargeAmount > totalNeeded) ? totalNeeded : rechargeAmount;

    // Calculate cost
    double totalCost = effectiveRecharge * costPerPoint;

    // Check if we can afford it and deduct cost if using external sync
    if (externalSync)
    {
        extern int32_t Cash;

        if ((int)totalCost > Cash)
        {
            // Calculate how much we can afford
            effectiveRecharge = Cash / costPerPoint;
            totalCost = effectiveRecharge * costPerPoint;

            if (effectiveRecharge <= 0.0)
            {
                printf("Insufficient credits for energy recharge.\n");
                return 0.0;
            }
        }

        // Deduct cost
        Cash -= (int)totalCost;
    }

    // Apply recharge prioritizing energy banks, then front shields, then aft shields
    double remainingCharge = effectiveRecharge;

    // Recharge energy banks first
    if (energyNeeded > 0.0)
    {
        double energyCharge = (remainingCharge > energyNeeded) ? energyNeeded : remainingCharge;
        playerShip->attributes.energyBanks += energyCharge;
        remainingCharge -= energyCharge;
    }

    // Then recharge shields if requested
    if (rechargeShields && remainingCharge > 0.0)
    {
        // Front shields
        if (shieldFrontNeeded > 0.0)
        {
            double frontCharge = (remainingCharge > shieldFrontNeeded) ? shieldFrontNeeded : remainingCharge;
            playerShip->attributes.shieldStrengthFront += frontCharge;
            remainingCharge -= frontCharge;
        }

        // Aft shields
        if (shieldAftNeeded > 0.0 && remainingCharge > 0.0)
        {
            double aftCharge = (remainingCharge > shieldAftNeeded) ? shieldAftNeeded : remainingCharge;
            playerShip->attributes.shieldStrengthAft += aftCharge;
            remainingCharge -= aftCharge;
        }
    }

    printf("Recharged %.1f energy units for %.0f credits.\n", effectiveRecharge, totalCost);
    return effectiveRecharge;
}

/**
 * Adds equipment to the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param equipmentName Name of the equipment
 * @param slotType The type of slot this equipment will be installed in
 * @param specificType A union containing the type-specific data
 * @param energyDraw How much energy this equipment draws
 * @param damageOutput How much damage this equipment does (for weapons)
 *
 * @return true if equipment was added successfully, false otherwise
 */
inline bool AddEquipment(PlayerShip *playerShip,
                         EquipmentSlotType slotType,
                         const char *equipmentName,
                         EquipmentTypeSpecifics specificType, // Changed to named union
                         double energyDraw,
                         double damageOutput)
{ // damageOutput is 0 for non-weapons
    if (playerShip == NULL || equipmentName == NULL)
    {
        return false;
    }    // Check if the slot is valid and available (or if we are replacing existing)
    // The slotType directly corresponds to the array index in the equipment array
    
    // Make sure the slot is within bounds
    if (slotType < 0 || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        printf("Error: Invalid equipment slot type %d.\n", slotType);
        return false;
    }    // Check if the slot is already occupied
    if (playerShip->equipment[slotType].isActive)
    {
        // Forward declaration of the inventory function
        extern bool RemoveEquipmentToInventory(PlayerShip* playerShip, EquipmentSlotType slotType);
        
        // Store the name of the equipment being replaced for better messaging
        char oldEquipName[MAX_SHIP_NAME_LENGTH];
        strncpy(oldEquipName, playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH - 1);
        oldEquipName[MAX_SHIP_NAME_LENGTH - 1] = '\0';
          // Try to store the existing equipment in inventory before replacing it
        if (RemoveEquipmentToInventory(playerShip, slotType)) {
            // Successfully moved existing equipment to inventory - message is already printed by RemoveEquipmentToInventory
        } else {
            // Failed to store in inventory - likely full or special case
            printf("Warning: Replacing existing equipment '%s' in slot %d without storing it (inventory may be full).\n", 
                   oldEquipName, slotType);
            
            // Reset the slot manually since RemoveEquipmentToInventory failed
            playerShip->equipment[slotType].isActive = 0;
            strcpy(playerShip->equipment[slotType].name, "Empty");
        }
    }// Add the equipment directly to the specified slot
    strncpy(playerShip->equipment[slotType].name, equipmentName, MAX_SHIP_NAME_LENGTH - 1);
    playerShip->equipment[slotType].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    playerShip->equipment[slotType].slotType = slotType;         // Store the intended slot type
    playerShip->equipment[slotType].typeSpecific = specificType; // Use the new named union
    playerShip->equipment[slotType].isActive = 1;
    playerShip->equipment[slotType].energyDraw = energyDraw;
    playerShip->equipment[slotType].damageOutput = damageOutput;

    printf("%s added to slot %d.\n", equipmentName, slotType);
    return true;
}

/**
 * Removes equipment from the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The slot to remove equipment from
 *
 * @return true if equipment was removed successfully, false otherwise
 */
inline bool RemoveEquipment(PlayerShip *playerShip, EquipmentSlotType slotType)
{
    if (playerShip == NULL || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        return false;
    }

    // Check if there's actually equipment installed
    if (!playerShip->equipment[slotType].isActive)
    {
        printf("Error: No equipment installed in slot %d.\n", slotType);
        return false;
    }

    // Save name for display
    char equipmentName[MAX_SHIP_NAME_LENGTH];
    strncpy(equipmentName, playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH - 1);
    equipmentName[MAX_SHIP_NAME_LENGTH - 1] = '\0';

    // Special handling before removal
    if (playerShip->equipment[slotType].slotType >= UTILITY_SYSTEM_1 &&
        playerShip->equipment[slotType].slotType <= UTILITY_SYSTEM_4)
    {

        // Reverse cargo bay extension effect
        if (playerShip->equipment[slotType].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION)
        {
            // Check if removing cargo capacity would leave enough space for current cargo
            if (playerShip->attributes.cargoCapacityTons - 5 < playerShip->attributes.currentCargoTons)
            {
                printf("Error: Can't remove cargo bay extension while cargo hold contains more than %d tons.\n",
                       playerShip->attributes.cargoCapacityTons - 5);
                return false;
            }

            // Decrease cargo capacity
            playerShip->attributes.cargoCapacityTons -= 5;
        }
    }

    // Reset the equipment slot
    playerShip->equipment[slotType].isActive = 0;
    strcpy(playerShip->equipment[slotType].name, "Empty");
    // Leave other fields as they are - they'll be overwritten on next install

    printf("Successfully removed %s from slot %d.\n", equipmentName, slotType);
    return true;
}

/**
 * Finds cargo by name in the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo item to find
 *
 * @return The index of the cargo item, or -1 if not found
 */
inline int FindCargoByName(const PlayerShip *playerShip, const char *cargoName)
{
    if (playerShip == NULL || cargoName == NULL)
    {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0 &&
            strcmp(playerShip->cargo[i].name, cargoName) == 0)
        {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the available cargo space in the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 *
 * @return The available cargo space in tons
 */
inline int GetAvailableCargoSpace(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    return playerShip->attributes.cargoCapacityTons - playerShip->attributes.currentCargoTons;
}

/**
 * Gets the total number of cargo items (different types) in the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 *
 * @return The total number of different cargo items
 */
inline int GetCargoItemCount(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            count++;
        }
    }

    return count;
}

/**
 * Gets the cargo item at a specific index.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param index The index of the cargo item
 * @param outCargoName Pointer to store the cargo name (must be at least MAX_SHIP_NAME_LENGTH)
 * @param outQuantity Pointer to store the quantity
 * @param outPurchasePrice Pointer to store the purchase price
 *
 * @return true if successful, false if index is out of range or no cargo at index
 */
inline bool GetCargoItemAtIndex(const PlayerShip *playerShip, int index,
                                char *outCargoName, int *outQuantity, int *outPurchasePrice)
{
    if (playerShip == NULL || outCargoName == NULL || outQuantity == NULL || outPurchasePrice == NULL)
    {
        return false;
    }

    if (index < 0 || index >= MAX_CARGO_SLOTS)
    {
        return false;
    }

    if (playerShip->cargo[index].quantity <= 0)
    {
        return false;
    }

    strncpy(outCargoName, playerShip->cargo[index].name, MAX_SHIP_NAME_LENGTH - 1);
    outCargoName[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    *outQuantity = playerShip->cargo[index].quantity;
    *outPurchasePrice = playerShip->cargo[index].purchasePrice;

    return true;
}

/**
 * Displays detailed information about the cargo in the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 */
inline void DisplayCargoDetails(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        printf("Error: Ship data is NULL.\n");
        return;
    }

    printf("\n=== Cargo Hold (%d/%d tons) ===\n",
           playerShip->attributes.currentCargoTons,
           playerShip->attributes.cargoCapacityTons);

    if (playerShip->attributes.currentCargoTons == 0)
    {
        printf("Cargo hold is empty.\n");
        return;
    }

    printf("%-20s %-10s %-15s %-15s\n", "Commodity", "Quantity", "Purchase Price", "Total Value");
    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");

    int totalItems = 0;
    int totalValue = 0;

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        if (playerShip->cargo[i].quantity > 0)
        {
            int itemTotalValue = playerShip->cargo[i].quantity * playerShip->cargo[i].purchasePrice;
            totalItems += playerShip->cargo[i].quantity;
            totalValue += itemTotalValue;

            printf("%-20s %-10d %-15d %-15d\n",
                   playerShip->cargo[i].name,
                   playerShip->cargo[i].quantity,
                   playerShip->cargo[i].purchasePrice,
                   itemTotalValue);
        }
    }

    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");
    printf("%-20s %-10d %-15s %-15d\n", "TOTAL", totalItems, "", totalValue);
    printf("\nAvailable space: %d tons\n", GetAvailableCargoSpace(playerShip));
}

// Helper functions to get equipment names from types
inline const char* GetWeaponTypeName(WeaponType type)
{
    switch (type)
    {
        case WEAPON_TYPE_PULSE_LASER:
            return "Pulse Laser";
        case WEAPON_TYPE_BEAM_LASER:
            return "Beam Laser";
        case WEAPON_TYPE_MILITARY_LASER:
            return "Military Laser";
        case WEAPON_TYPE_MINING_LASER:
            return "Mining Laser";
        case WEAPON_TYPE_MISSILE_HOMING:
            return "Homing Missile";
        case WEAPON_TYPE_MISSILE_DUMBFIRE:
            return "Dumbfire Missile";
        case WEAPON_TYPE_REAR_LASER:
            return "Rear Laser";
        case WEAPON_TYPE_NONE:
        default:
            return "None";
    }
}

inline const char* GetDefensiveSystemTypeName(DefensiveSystemType type)
{
    switch (type)
    {
        case DEFENSIVE_SYSTEM_TYPE_ECM:
            return "ECM System";
        case DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT:
            return "Extra Energy Unit";
        case DEFENSIVE_SYSTEM_TYPE_NONE:
        default:
            return "None";
    }
}

inline const char* GetUtilitySystemTypeName(UtilitySystemType type)
{
    switch (type)
    {
        case UTILITY_SYSTEM_TYPE_ESCAPE_POD:
            return "Escape Pod";
        case UTILITY_SYSTEM_TYPE_FUEL_SCOOPS:
            return "Fuel Scoops";
        case UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION:
            return "Cargo Bay Extension";
        case UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER:
            return "Docking Computer";
        case UTILITY_SYSTEM_TYPE_GALACTIC_HYPERSPACE_DRIVE:
            return "Galactic Hyperspace Drive";
        case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
            return "Scanner Upgrade";
        case UTILITY_SYSTEM_TYPE_NONE:
        default:
            return "None";
    }
}
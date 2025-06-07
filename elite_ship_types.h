#pragma once

#include <stddef.h>  // For NULL
#include <stdint.h>  // For uint16_t, int32_t
#include <stdbool.h> // For bool
#include <string.h>  // For string functions
#include <stdio.h>   // For printf

// Forward declaration
typedef struct ShipType ShipType;

// --- Constants for Equipment Types ---
#define MAX_SHIP_NAME_LENGTH 64
#define MAX_EQUIPMENT_SLOTS 10 // Max possible equipment slots
#define MAX_CARGO_SLOTS 50     // Max types of cargo a ship can hold

// Backward compatibility constants for Cobra Mk III
// These will be used in places where the code still references the old constants
#define COBRA_MK3_BASE_HULL_STRENGTH 100
#define COBRA_MK3_BASE_SHIELD_STRENGTH 50.0
#define COBRA_MK3_MAX_FUEL_LY 7.0
#define COBRA_MK3_BASE_CARGO_CAPACITY_TONS 20

/**
 * Structure defining a ship type with its base specifications
 */
typedef struct ShipType
{
    char className[MAX_SHIP_NAME_LENGTH]; // e.g., "Cobra Mk III"
    int baseHullStrength;                 // Base hull strength
    double baseShieldStrengthFront;       // Base front shield strength
    double baseShieldStrengthAft;         // Base aft shield strength
    double maxFuelLY;                     // Maximum fuel capacity in LY
    double fuelConsumptionRate;           // Fuel consumption rate (lower is better, liters per 0.1 LY)
    int baseCargoCapacityTons;            // Base cargo capacity in tons
    int initialMissilePylons;             // Initial missile pylons
    double baseCost;                      // Base cost in credits
    int baseSpeed;                        // Base speed
    int baseManeuverability;              // Base maneuverability (higher is better)
    int defaultWeaponSlots;               // Number of default weapon slots
    int defaultDefensiveSlots;            // Number of default defensive slots
    int defaultUtilitySlots;              // Number of default utility slots
    bool hasStandardHyperdrive;           // Whether ship has a standard hyperdrive
    bool hasStandardShields;              // Whether ship has standard shields
    bool includesPulseLaser;              // Whether ship comes with a pulse laser
} ShipType;

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
    DEFENSIVE_SYSTEM_TYPE_ECM              // Electronic Counter-Measures
} DefensiveSystemType;

typedef enum UtilitySystemType
{
    UTILITY_SYSTEM_TYPE_NONE,
    UTILITY_SYSTEM_TYPE_ESCAPE_POD,
    UTILITY_SYSTEM_TYPE_FUEL_SCOOPS,
    UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION, // Represents the upgrade itself
    UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER,
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

// Maximum number of ship types that can be registered
#define MAX_SHIP_TYPES 32

/**
 * Ship registry to store all available ship types
 */
typedef struct ShipRegistry
{
    ShipType shipTypes[MAX_SHIP_TYPES]; // Array to store all ship types
    int registeredShipCount;            // Number of registered ship types
} ShipRegistry;

// The global ship registry
static ShipRegistry shipRegistry = {.registeredShipCount = 0};

/**
 * Register a new ship type in the registry
 *
 * @param className Name of the ship class
 * @param baseHullStrength Base hull strength
 * @param baseShieldStrengthFront Base front shield strength
 * @param baseShieldStrengthAft Base aft shield strength
 * @param maxFuelLY Maximum fuel capacity in LY
 * @param baseCargoCapacityTons Base cargo capacity in tons
 * @param initialMissilePylons Initial missile pylons
 * @param baseCost Base cost in credits
 * @param baseSpeed Base speed
 * @param baseManeuverability Base maneuverability (higher is better)
 * @param defaultWeaponSlots Number of default weapon slots
 * @param defaultDefensiveSlots Number of default defensive slots
 * @param defaultUtilitySlots Number of default utility slots
 * @param hasStandardHyperdrive Whether ship has a standard hyperdrive
 * @param hasStandardShields Whether ship has standard shields
 * @param includesPulseLaser Whether ship comes with a pulse laser
 *
 * @return Pointer to the registered ship type, or NULL if registration failed
 */
static inline const ShipType *RegisterShipType(
    const char *className,
    int baseHullStrength,
    double baseShieldStrengthFront,
    double baseShieldStrengthAft,
    double maxFuelLY,
    double fuelConsumptionRate,
    int baseCargoCapacityTons,
    int initialMissilePylons,
    double baseCost,
    int baseSpeed,
    int baseManeuverability,
    int defaultWeaponSlots,
    int defaultDefensiveSlots,
    int defaultUtilitySlots,
    bool hasStandardHyperdrive,
    bool hasStandardShields,
    bool includesPulseLaser)
{
    // Check if we have space for a new ship type
    if (shipRegistry.registeredShipCount >= MAX_SHIP_TYPES)
    {
        printf("Error: Cannot register more ship types. Maximum limit reached.\n");
        return NULL;
    }

    // Get a reference to the new ship type slot
    ShipType *newShipType = &shipRegistry.shipTypes[shipRegistry.registeredShipCount];    // Initialize the new ship type with provided values
    snprintf(newShipType->className, MAX_SHIP_NAME_LENGTH, "%s", className);
    newShipType->className[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    newShipType->baseHullStrength = baseHullStrength;
    newShipType->baseShieldStrengthFront = baseShieldStrengthFront;
    newShipType->baseShieldStrengthAft = baseShieldStrengthAft;
    newShipType->maxFuelLY = maxFuelLY;
    newShipType->fuelConsumptionRate = fuelConsumptionRate;
    newShipType->baseCargoCapacityTons = baseCargoCapacityTons;
    newShipType->initialMissilePylons = initialMissilePylons;
    newShipType->baseCost = baseCost;
    newShipType->baseSpeed = baseSpeed;
    newShipType->baseManeuverability = baseManeuverability;
    newShipType->defaultWeaponSlots = defaultWeaponSlots;
    newShipType->defaultDefensiveSlots = defaultDefensiveSlots;
    newShipType->defaultUtilitySlots = defaultUtilitySlots;
    newShipType->hasStandardHyperdrive = hasStandardHyperdrive;
    newShipType->hasStandardShields = hasStandardShields;
    newShipType->includesPulseLaser = includesPulseLaser;

    // Increment the counter
    shipRegistry.registeredShipCount++;

    // Return a pointer to the newly registered ship type
    return newShipType;
}

/**
 * Initialize the ship registry with predefined ship types
 */
static inline void InitializeShipRegistry(void)
{
    // Only initialize if the registry is empty
    if (shipRegistry.registeredShipCount > 0)
    {
        return;    } // Register Cobra Mk III
    RegisterShipType(
        "Cobra Mk III", // className
        100,            // baseHullStrength
        50.0,           // baseShieldStrengthFront
        50.0,           // baseShieldStrengthAft
        7.0,            // maxFuelLY
        2.0,            // fuelConsumptionRate (liters per 0.1 LY)
        20,             // baseCargoCapacityTons
        0,              // initialMissilePylons
        10000.0,        // baseCost
        30,             // baseSpeed
        4,              // baseManeuverability
        1,              // defaultWeaponSlots
        1,              // defaultDefensiveSlots
        2,              // defaultUtilitySlots
        true,           // hasStandardHyperdrive
        true,           // hasStandardShields
        true            // includesPulseLaser
    );    // Register Viper
    RegisterShipType(
        "Viper", // className
        80,      // baseHullStrength
        40.0,    // baseShieldStrengthFront
        40.0,    // baseShieldStrengthAft
        5.0,     // maxFuelLY
        1.5,     // fuelConsumptionRate (liters per 0.1 LY) - more efficient than Cobra
        10,      // baseCargoCapacityTons
        2,       // initialMissilePylons
        8000.0,  // baseCost
        40,      // baseSpeed
        6,       // baseManeuverability
        2,       // defaultWeaponSlots
        1,       // defaultDefensiveSlots
        1,       // defaultUtilitySlots
        true,    // hasStandardHyperdrive
        true,    // hasStandardShields
        true     // includesPulseLaser
    );    // Register Asp Mk II
    RegisterShipType(
        "Asp Mk II", // className
        120,         // baseHullStrength
        60.0,        // baseShieldStrengthFront
        60.0,        // baseShieldStrengthAft
        8.0,         // maxFuelLY
        2.5,         // fuelConsumptionRate (liters per 0.1 LY) - less efficient, larger ship
        30,          // baseCargoCapacityTons
        1,           // initialMissilePylons
        15000.0,     // baseCost
        25,          // baseSpeed
        3,           // baseManeuverability
        2,           // defaultWeaponSlots
        2,           // defaultDefensiveSlots
        2,           // defaultUtilitySlots
        true,        // hasStandardHyperdrive
        true,        // hasStandardShields
        true         // includesPulseLaser
    );
}

/**
 * Get a pointer to a ship type by its class name
 *
 * @param className The class name of the ship type to find
 * @return Pointer to the ShipType, or NULL if not found
 */
static inline const ShipType *GetShipTypeByName(const char *className)
{
    if (className == NULL)
    {
        return NULL;
    }

    // Search through the registry for a matching ship type
    for (int i = 0; i < shipRegistry.registeredShipCount; i++)
    {
        if (strcmp(className, shipRegistry.shipTypes[i].className) == 0)
        {
            return &shipRegistry.shipTypes[i];
        }
    }

    return NULL;
}

// Forward declaration for MAX_EQUIPMENT_INVENTORY
#ifndef MAX_EQUIPMENT_INVENTORY
#define MAX_EQUIPMENT_INVENTORY 30
#endif

typedef struct PlayerShip
{
    char shipName[MAX_SHIP_NAME_LENGTH];
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // e.g., "Cobra Mk III"
    const ShipType *shipType;                 // Pointer to the ship type definition
    ShipCoreAttributes attributes;
    ShipEquipmentItem equipment[MAX_EQUIPMENT_SLOTS];              // Currently equipped items
    ShipEquipmentItem equipmentInventory[MAX_EQUIPMENT_INVENTORY]; // Inventory of stored equipment
    CargoItem cargo[MAX_CARGO_SLOTS];
} PlayerShip;

// --- Function Prototypes (Implementations will be in this header as per standard) ---

// Initializes a PlayerShip with the given ship type
//
// @param playerShip Pointer to the PlayerShip structure to initialize
// @param shipType Pointer to the ShipType to use
// @param customName Custom name for the ship (or NULL to use default)
// @return true if successful, false otherwise
static inline bool InitializeShip(PlayerShip *playerShip, const ShipType *shipType, const char *customName)
{
    if (playerShip == NULL || shipType == NULL)
    {
        return false;
    }

    // Set ship name (custom or default)
    if (customName != NULL && customName[0] != '\0')
    {
        snprintf(playerShip->shipName, MAX_SHIP_NAME_LENGTH, "%s", customName);
        playerShip->shipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    }
    else
    {
        // Construct default name if no custom name is provided
        char defaultName[MAX_SHIP_NAME_LENGTH];
        snprintf(defaultName, sizeof(defaultName), "%s Class", shipType->className);
        snprintf(playerShip->shipName, MAX_SHIP_NAME_LENGTH, "%s", defaultName);
        playerShip->shipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    }
    snprintf(playerShip->shipClassName, MAX_SHIP_NAME_LENGTH, "%s", shipType->className);
    playerShip->shipClassName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination

    // Set the ship type pointer
    playerShip->shipType = shipType;    // Initialize core attributes based on ship type
    playerShip->attributes.hullStrength = shipType->baseHullStrength;
    playerShip->attributes.shieldStrengthFront = shipType->baseShieldStrengthFront;
    playerShip->attributes.shieldStrengthAft = shipType->baseShieldStrengthAft;
    playerShip->attributes.fuelLiters = shipType->maxFuelLY * 100.0; // Assuming 100 liters per LY
    playerShip->attributes.cargoCapacityTons = shipType->baseCargoCapacityTons;
    playerShip->attributes.currentCargoTons = 0;
    playerShip->attributes.missilePylons = shipType->initialMissilePylons;
    playerShip->attributes.missilesLoadedHoming = 0;
    playerShip->attributes.missilesLoadedDumbfire = 0;

    // Initialize equipment slots to Empty
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        playerShip->equipment[i].isActive = false;
        snprintf(playerShip->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        playerShip->equipment[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        playerShip->equipment[i].typeSpecific.utilityType = UTILITY_SYSTEM_TYPE_NONE; // Example default
    }    // Initialize inventory slots to empty
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i)
    {
        playerShip->equipmentInventory[i].isActive = 0;
        snprintf(playerShip->equipmentInventory[i].name, MAX_SHIP_NAME_LENGTH, "Empty");        playerShip->equipmentInventory[i].slotType = EQUIPMENT_SLOT_TYPE_NONE;
        playerShip->equipmentInventory[i].damageOutput = 0.0;
    }

    // Initialize cargo holds to Empty
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i)
    {
        snprintf(playerShip->cargo[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        playerShip->cargo[i].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        playerShip->cargo[i].quantity = 0;
    }

    // Add pulse laser if the ship type includes one
    if (shipType->includesPulseLaser)
    {
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].isActive = true;
        snprintf(playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name, MAX_SHIP_NAME_LENGTH, "Pulse Laser");
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination        // Set other properties for Pulse Laser as needed
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].typeSpecific.weaponType = WEAPON_TYPE_PULSE_LASER;
        playerShip->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].damageOutput = 5.0; // Example
    }

    return true;
}

// Initializes a PlayerShip to Cobra Mk III default specifications.
static inline void InitializeCobraMkIII(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    // Ensure the ship registry is initialized
    InitializeShipRegistry();

    // Get the Cobra Mk III ship type from the registry
    const ShipType *cobraMkIII = GetShipTypeByName("Cobra Mk III");
    if (cobraMkIII == NULL)
    {
        printf("Error: Could not find Cobra Mk III ship type in registry.\n");
        return;
    }

    InitializeShip(playerShip, cobraMkIII, NULL);
}

// Displays the current status of the player's ship.
static inline void DisplayShipStatus(const PlayerShip *playerShip)
{    bool ecmFound = false; // Moved declaration here
    bool escapePodFound = false;
    bool fuelScoopsFound = false;
    bool dockingComputerFound = false;
    bool scannerUpgradeFound = false;
    bool rearLaserFound = false;
    bool forwardPulseLaserFound = false;

    if (playerShip == NULL)
    {
        printf("Error: Ship data is NULL.\n");
        return;
    }    printf("\n--- %s (%s) Status ---\n", playerShip->shipName, playerShip->shipClassName);
    printf("Hull Strength: %d / %d\n", playerShip->attributes.hullStrength, playerShip->shipType->baseHullStrength);
    printf("Shields (F/A): %.2f / %.2f\n", playerShip->attributes.shieldStrengthFront, playerShip->attributes.shieldStrengthAft);
    // Convert Liters to LY for display, assuming 1 LY = 100 Liters (example factor)
    printf("Fuel: %.2f LY (%.0f Liters)\n", playerShip->attributes.fuelLiters / 100.0, playerShip->attributes.fuelLiters);
    printf("Cargo: %dT / %dT\n", playerShip->attributes.currentCargoTons, playerShip->attributes.cargoCapacityTons);
    printf("Missile Pylons: %d (Homing: %d, Dumbfire: %d)\n",
           playerShip->attributes.missilePylons,
           playerShip->attributes.missilesLoadedHoming,
           playerShip->attributes.missilesLoadedDumbfire);
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
            if (playerShip->equipment[i].slotType != EQUIPMENT_SLOT_TYPE_NONE)
            {
                printf(" (Slot: %d", playerShip->equipment[i].slotType);

                // Print the type info based on slot type
                if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
                    playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
                {
                    printf(", Type: Weapon - %d", playerShip->equipment[i].typeSpecific.weaponType);
                }
                else if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                         playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
                {
                    printf(", Type: Defensive - %d", playerShip->equipment[i].typeSpecific.defensiveType);
                }
                else if (playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                         playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4)
                {
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
    } // Display standard equipment flags for clarity based on design doc
    // This section is now dynamic based on equipment array and ship class
    printf("\n--- Key Systems & Upgrades ---\n");

    bool isCobraMkIII = (strcmp(playerShip->shipClassName, "Cobra Mk III") == 0); // Standard Inherent features for Cobra Mk III
    if (isCobraMkIII)
    {
        printf("- Basic Shields System\n"); // Cobra Mk III always has shields
    }

    // Display fuel-related information for all ships
    printf("- %s Hyperspace Drive (%.1f LY Max, %.1f CR per 0.1 LY)\n",
           playerShip->shipType->hasStandardHyperdrive ? "Standard" : "Enhanced",
           playerShip->shipType->maxFuelLY,
           playerShip->shipType->fuelConsumptionRate / 10.0);

    // Standard Cargo Bay is reflected in attributes.cargoCapacityTons
    printf("- Standard Cargo Bay (%dT)\n", playerShip->shipType->baseCargoCapacityTons);

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
            }            // Defensive Systems
            if (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
            {
                if (playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM && !ecmFound)
                {
                    printf("- ECM Unit\n");
                    ecmFound = true;
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
    }
    if (!hasCargo)
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
static inline bool HasFuelScoops(const PlayerShip *playerShip)
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
static inline float RefuelShip(PlayerShip *playerShip, float fuelAmountLY, bool useFuelScoops, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0.0f;
    }

    // Check if we're already at maximum fuel
    const float maxFuelLY = playerShip->shipType->maxFuelLY;
    float currentFuelLY = playerShip->attributes.fuelLiters / 100.0f; // Convert liters to LY assuming 100L = 1LY

    if (currentFuelLY >= maxFuelLY)
    {
        printf("Fuel tanks already full (%.1f LY).\n", maxFuelLY);
        return 0.0f;
    }

    // Calculate how much more fuel can fit in the tank
    float availableSpace = maxFuelLY - currentFuelLY;

    // Limit requested amount to available space
    float effectiveRequestLY = (fuelAmountLY > availableSpace) ? availableSpace : fuelAmountLY; // Handle fuel scooping if requested
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
        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f);                  // Convert LY to liters
        printf("Successfully scooped %.1f LY of fuel from the star.\n", effectiveRequestLY); // Sync with global state if requested
        if (externalSync)
        { // Declare external variables
            extern uint16_t Fuel;

            // Import the function from elite_state.h
            extern int GetMaxFuel(void);

            // Get the current ship's max fuel
            int currentMaxFuel = GetMaxFuel();

            // Convert to the units used in elite_state.h (tenth of LY)
            uint16_t fuelToAdd = (uint16_t)(effectiveRequestLY * 10.0f);

            // Make sure not to exceed the ship's max fuel
            if (Fuel + fuelToAdd > (uint16_t)currentMaxFuel)
            {
                Fuel = (uint16_t)currentMaxFuel;
            }
            else
            {
                Fuel += fuelToAdd;
            }
        }

        return effectiveRequestLY;
    } // Standard refueling at a station (costs money)
    else
    { // Declare external variables
        extern int32_t Cash;

        // Import the function from elite_state.h
        extern int GetFuelCost(void);

        // Get the current fuel cost based on ship type
        int currentFuelCost = GetFuelCost();

        // Convert to tenths of LY for cost calculation
        uint16_t fuelUnits = (uint16_t)(effectiveRequestLY * 10.0f);
        int totalCost = fuelUnits * currentFuelCost;

        // Check if we can afford it
        if (externalSync && totalCost > Cash)
        {
            // Calculate how much we can afford
            uint16_t affordableUnits = (uint16_t)(Cash / currentFuelCost);
            fuelUnits = affordableUnits;
            totalCost = fuelUnits * currentFuelCost;
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
        } // Add the fuel to the ship
        playerShip->attributes.fuelLiters += (effectiveRequestLY * 100.0f); // Convert LY to liters        // Sync with global state if requested
        if (externalSync)
        {
            extern uint16_t Fuel;

            // Import the function from elite_state.h
            extern int GetMaxFuel(void);

            // Get the current ship's max fuel
            int currentMaxFuel = GetMaxFuel();

            // Make sure not to exceed the ship's max fuel
            if (Fuel + fuelUnits > (uint16_t)currentMaxFuel)
            {
                Fuel = (uint16_t)currentMaxFuel;
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
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if ECM was successfully activated, false otherwise
 */
static inline bool ActivateECM(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has ECM
    bool hasECM = false;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
             playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM)
        {
            hasECM = true;
            break;
        }
    }

    if (!hasECM)
    {
        printf("Error: Your ship is not equipped with ECM System.\n");
        return false;
    }

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
static inline bool ActivateDockingComputer(PlayerShip *playerShip, double distance)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has docking computer
    bool hasDockingComputer = false;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER)
        {
            hasDockingComputer = true;
            break;
        }
    }

    if (!hasDockingComputer)
    {
        printf("Error: Your ship is not equipped with a Docking Computer.\n");
        return false;
    }

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
static inline bool UseScanner(PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Check if ship has advanced scanner
    bool hasUpgradedScanner = false;

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        if (playerShip->equipment[i].isActive &&
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE)
        {
            hasUpgradedScanner = true;
            break;
        }
    }

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
static inline bool DeployEscapePod(PlayerShip *playerShip, bool criticalDamage)
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
 * Gets the damage output of a specific weapon.
 * Used in combat calculations to determine damage dealt to targets.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The weapon slot to check (forward or aft)
 * @return The damage output value, or 0.0 if no weapon is installed
 */
static inline double GetWeaponDamage(const PlayerShip *playerShip, EquipmentSlotType slotType)
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

// Function to check if a specific type of equipment is installed
/**
 * Checks if the ship has a specific type of equipment installed.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The type of slot to check (e.g., EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON)
 * @param specificType The specific type to check (EquipmentTypeSpecifics union)
 * @return true if the equipment is installed, false otherwise
 */
static inline bool HasEquipment(const PlayerShip *playerShip, EquipmentSlotType slotType, EquipmentTypeSpecifics specificType)
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
static inline int RepairHull(PlayerShip *playerShip, int repairAmount, int costPerPoint, bool externalSync)
{
    if (playerShip == NULL)
    {
        return 0;
    }

    // Check if hull is already at maximum
    if (playerShip->attributes.hullStrength >= playerShip->shipType->baseHullStrength)
    {
        printf("Hull already at maximum strength.\n");
        return 0;
    }

    // Calculate how much more hull strength can be repaired
    int maxRepair = playerShip->shipType->baseHullStrength - playerShip->attributes.hullStrength;

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
 * Adds equipment to the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param equipmentName Name of the equipment
 * @param slotType The type of slot this equipment will be installed in
 * @param specificType A union containing the type-specific data
 * @param damageOutput How much damage this equipment does (for weapons)
 *
 * @return true if equipment was added successfully, false otherwise
 */
static inline bool AddEquipment(PlayerShip *playerShip,
                         EquipmentSlotType slotType,
                         const char *equipmentName,
                         EquipmentTypeSpecifics specificType, // Changed to named union
                         double damageOutput)
{ // damageOutput is 0 for non-weapons
    if (playerShip == NULL || equipmentName == NULL)
    {
        return false;
    } // Check if the slot is valid and available (or if we are replacing existing)
    // The slotType directly corresponds to the array index in the equipment array

    // Make sure the slot is within bounds
    if (slotType < 0 || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        printf("Error: Invalid equipment slot type %d.\n", slotType);
        return false;
    } // Check if the slot is already occupied
    if (playerShip->equipment[slotType].isActive)
    {
        // Store the name of the equipment being replaced, if any
        char oldEquipName[MAX_SHIP_NAME_LENGTH];
        if (playerShip->equipment[slotType].isActive)
        {
            snprintf(oldEquipName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->equipment[slotType].name);
            oldEquipName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
        }
        // Try to store the existing equipment in inventory before replacing it
        if (RemoveEquipmentToInventory(playerShip, slotType))
        {
            // Successfully moved existing equipment to inventory - message is already printed by RemoveEquipmentToInventory
        }
        else
        {
            // Failed to store in inventory - likely full or special case
            printf("Warning: Replacing existing equipment '%s' in slot %d without storing it (inventory may be full).\n",
                   oldEquipName, slotType);            // Reset the slot manually since RemoveEquipmentToInventory failed
            playerShip->equipment[slotType].isActive = 0;
            snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "Empty");
        }
    }    // Install the new equipment
    playerShip->equipment[slotType].isActive = true;
    snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "%s", equipmentName);
    playerShip->equipment[slotType].name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    playerShip->equipment[slotType].typeSpecific = specificType;
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
static inline bool RemoveEquipment(PlayerShip *playerShip, EquipmentSlotType slotType)
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

    // Save current equipment name before removal
    char equipmentName[MAX_SHIP_NAME_LENGTH];
    snprintf(equipmentName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->equipment[slotType].name);
    equipmentName[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure null termination

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
    }    // Reset the equipment slot
    playerShip->equipment[slotType].isActive = 0;
    snprintf(playerShip->equipment[slotType].name, MAX_SHIP_NAME_LENGTH, "Empty");
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
static inline int FindCargoByName(const PlayerShip *playerShip, const char *cargoName)
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
static inline int GetAvailableCargoSpace(const PlayerShip *playerShip)
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
static inline int GetCargoItemCount(const PlayerShip *playerShip)
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
static inline bool GetCargoItemAtIndex(const PlayerShip *playerShip, int index,
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

    snprintf(outCargoName, MAX_SHIP_NAME_LENGTH, "%s", playerShip->cargo[index].name);
    // outCargoName[MAX_SHIP_NAME_LENGTH - 1] = '\\0'; // snprintf handles null termination
    *outQuantity = playerShip->cargo[index].quantity;
    *outPurchasePrice = playerShip->cargo[index].purchasePrice;

    return true;
}

/**
 * Displays detailed information about the cargo in the player's ship.
 *
 * @param playerShip Pointer to the PlayerShip structure
 */
static inline void DisplayCargoDetails(const PlayerShip *playerShip)
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
static inline const char *GetWeaponTypeName(WeaponType type)
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

static inline const char *GetDefensiveSystemTypeName(DefensiveSystemType type)
{
    switch (type)
    {
    case DEFENSIVE_SYSTEM_TYPE_ECM:
        return "ECM System";
    case DEFENSIVE_SYSTEM_TYPE_NONE:
    default:
        return "None";
    }
}

static inline const char *GetUtilitySystemTypeName(UtilitySystemType type)
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
    case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
        return "Scanner Upgrade";
    case UTILITY_SYSTEM_TYPE_NONE:
    default:
        return "None";
    }
}
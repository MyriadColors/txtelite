#pragma once

#include "elite_ship_components.h"
#include <stdio.h>
#include <string.h>

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

// Maximum number of ship types that can be registered
#ifndef MAX_SHIP_TYPES
#define MAX_SHIP_TYPES 32
#endif

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
    ShipType *newShipType = &shipRegistry.shipTypes[shipRegistry.registeredShipCount];
    
    // Initialize the new ship type with provided values
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

/**
 * Initialize the ship registry with predefined ship types
 */
static inline void InitializeShipRegistry(void)
{
    // Only initialize if the registry is empty
    if (shipRegistry.registeredShipCount > 0)
    {
        return;
    }

    // Register Cobra Mk III
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
        1,           // hasStandardHyperdrive
        1,           // hasStandardShields
        1            // includesPulseLaser
    );

    // Register Viper
    RegisterShipType(
        "Viper", // className
        80,      // baseHullStrength
        40.0,    // baseShieldStrengthFront
        40.0,    // baseShieldStrengthAft
        5.0,     // maxFuelLY
        1.5,     // fuelConsumptionRate (liters per 0.1 LY)
        10,      // baseCargoCapacityTons
        2,       // initialMissilePylons
        8000.0,  // baseCost
        40,      // baseSpeed
        6,       // baseManeuverability
        2,       // defaultWeaponSlots
        1,       // defaultDefensiveSlots
        1,       // defaultUtilitySlots
        1,    // hasStandardHyperdrive
        1,    // hasStandardShields
        1     // includesPulseLaser
    );

    // Register Asp Mk II
    RegisterShipType(
        "Asp Mk II", // className
        120,         // baseHullStrength
        60.0,        // baseShieldStrengthFront
        60.0,        // baseShieldStrengthAft
        8.0,         // maxFuelLY
        2.5,         // fuelConsumptionRate (liters per 0.1 LY)
        30,          // baseCargoCapacityTons
        1,           // initialMissilePylons
        15000.0,     // baseCost
        25,          // baseSpeed
        3,           // baseManeuverability
        2,           // defaultWeaponSlots
        2,           // defaultDefensiveSlots
        2,           // defaultUtilitySlots
        1,        // hasStandardHyperdrive
        1,        // hasStandardShields
        1         // includesPulseLaser
    );

    // Register Sidewinder
    RegisterShipType(
        "Sidewinder", // className
        50,           // baseHullStrength
        30.0,         // baseShieldStrengthFront
        30.0,         // baseShieldStrengthAft
        5.0,          // maxFuelLY
        1.0,          // fuelConsumptionRate (liters per 0.1 LY)
        10,           // baseCargoCapacityTons
        1,            // initialMissilePylons
        4000.0,       // baseCost
        25,           // baseSpeed
        5,            // baseManeuverability
        1,            // defaultWeaponSlots
        1,            // defaultDefensiveSlots
        1,            // defaultUtilitySlots
        1,            // hasStandardHyperdrive
        1,            // hasStandardShields
        1             // includesPulseLaser
    );

    // Register Python
    RegisterShipType(
        "Python",   // className
        200,        // baseHullStrength
        80.0,       // baseShieldStrengthFront
        80.0,       // baseShieldStrengthAft
        7.0,        // maxFuelLY
        3.0,        // fuelConsumptionRate (liters per 0.1 LY)
        100,        // baseCargoCapacityTons
        4,          // initialMissilePylons
        50000.0,    // baseCost
        20,         // baseSpeed
        2,          // baseManeuverability
        4,          // defaultWeaponSlots
        2,          // defaultDefensiveSlots
        3,          // defaultUtilitySlots
        1,          // hasStandardHyperdrive
        1,          // hasStandardShields
        1           // includesPulseLaser
    );

    // Register Anaconda
    RegisterShipType(
        "Anaconda", // className
        300,        // baseHullStrength
        100.0,      // baseShieldStrengthFront
        100.0,      // baseShieldStrengthAft
        7.0,        // maxFuelLY
        4.0,        // fuelConsumptionRate (liters per 0.1 LY)
        800,        // baseCargoCapacityTons
        4,          // initialMissilePylons
        150000.0,   // baseCost
        15,         // baseSpeed
        1,          // baseManeuverability
        4,          // defaultWeaponSlots
        3,          // defaultDefensiveSlots
        4,          // defaultUtilitySlots
        1,          // hasStandardHyperdrive
        1,          // hasStandardShields
        1           // includesPulseLaser
    );

    // Register Fer-de-Lance
    RegisterShipType(
        "Fer-de-Lance", // className
        150,            // baseHullStrength
        80.0,           // baseShieldStrengthFront
        80.0,           // baseShieldStrengthAft
        7.0,            // maxFuelLY
        2.5,            // fuelConsumptionRate (liters per 0.1 LY)
        20,             // baseCargoCapacityTons
        4,              // initialMissilePylons
        100000.0,       // baseCost
        35,             // baseSpeed
        5,              // baseManeuverability
        4,              // defaultWeaponSlots
        2,              // defaultDefensiveSlots
        2,              // defaultUtilitySlots
        1,              // hasStandardHyperdrive
        1,              // hasStandardShields
        1               // includesPulseLaser
    );
}

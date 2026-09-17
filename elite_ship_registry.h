#pragma once

#include "elite_ship_components.h"
#include <stdio.h>
#include <string.h>

/**
 * Structure defining a ship type with its base specifications
 */
typedef struct ship_type_t {
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
} ship_type_t;

// Maximum number of ship types that can be registered
#ifndef MAX_SHIP_TYPES
#define MAX_SHIP_TYPES 32
#endif

/**
 * Ship registry to store all available ship types
 */
typedef struct ship_registry_t {
    ship_type_t ship_type_ts[MAX_SHIP_TYPES]; // Array to store all ship types
    int registeredShipCount;                  // Number of registered ship types
} ship_registry_t;

// The global ship registry
static ship_registry_t g_ship_registry = {.registeredShipCount = 0};

/**
 * Register a new ship type in the registry
 *
 * @return Pointer to the registered ship type, or nullptr if registration failed
 */
static inline const ship_type_t *register_ship_type_t(
    const char *class_name, int base_hull_strength, double base_shield_strength_front, double base_shield_strength_aft,
    double max_fuel_ly, double fuel_consumption_rate, int base_cargo_capacity_tons, int initial_missile_pylons,
    double base_cost, int base_speed, int base_maneuverability, int default_weapon_slots, int default_defensive_slots,
    int default_utility_slots, bool has_standard_hyperdrive, bool has_standard_shields, bool includes_pulse_laser) {
    // Check if we have space for a new ship type
    if (g_ship_registry.registeredShipCount >= MAX_SHIP_TYPES) {
        printf("Error: Cannot register more ship types. Maximum limit reached.\n");
        return nullptr;
    }

    // Get a reference to the new ship type slot
    ship_type_t *newship_type_t = &g_ship_registry.ship_type_ts[g_ship_registry.registeredShipCount];

    // Initialize the new ship type with provided values
    int written = snprintf(newship_type_t->className, MAX_SHIP_NAME_LENGTH, "%s", class_name);
    if (written < 0) {
        return nullptr;
    }
    newship_type_t->className[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure nullptr termination
    newship_type_t->baseHullStrength = base_hull_strength;
    newship_type_t->baseShieldStrengthFront = base_shield_strength_front;
    newship_type_t->baseShieldStrengthAft = base_shield_strength_aft;
    newship_type_t->maxFuelLY = max_fuel_ly;
    newship_type_t->fuelConsumptionRate = fuel_consumption_rate;
    newship_type_t->baseCargoCapacityTons = base_cargo_capacity_tons;
    newship_type_t->initialMissilePylons = initial_missile_pylons;
    newship_type_t->baseCost = base_cost;
    newship_type_t->baseSpeed = base_speed;
    newship_type_t->baseManeuverability = base_maneuverability;
    newship_type_t->defaultWeaponSlots = default_weapon_slots;
    newship_type_t->defaultDefensiveSlots = default_defensive_slots;
    newship_type_t->defaultUtilitySlots = default_utility_slots;
    newship_type_t->hasStandardHyperdrive = has_standard_hyperdrive;
    newship_type_t->hasStandardShields = has_standard_shields;
    newship_type_t->includesPulseLaser = includes_pulse_laser;

    // Increment the counter
    g_ship_registry.registeredShipCount++;

    // Return a pointer to the newly registered ship type
    return newship_type_t;
}

/**
 * Get a pointer to a ship type by its class name
 *
 * @param className The class name of the ship type to find
 * @return Pointer to the ship_type_t, or nullptr if not found
 */
[[maybe_unused]] static inline const ship_type_t *get_ship_type_t_by_name(const char *class_name) {
    if (class_name == nullptr) {
        return nullptr;
    }

    // Search through the registry for a matching ship type
    for (int i = 0; i < g_ship_registry.registeredShipCount; i++) {
        if (strcmp(class_name, g_ship_registry.ship_type_ts[i].className) == 0) {
            return &g_ship_registry.ship_type_ts[i];
        }
    }

    return nullptr;
}

/**
 * Initialize the ship registry with predefined ship types
 */
[[maybe_unused]] static inline void intialize_ship_registry_t(void) {
    // Only initialize if the registry is empty
    if (g_ship_registry.registeredShipCount > 0) {
        return;
    }

    // Register Cobra Mk III
    register_ship_type_t("Cobra Mk III", // className
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
    );

    // Register Viper
    register_ship_type_t("Viper", // className
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
                         true,    // hasStandardHyperdrive
                         true,    // hasStandardShields
                         true     // includesPulseLaser
    );

    // Register Asp Mk II
    register_ship_type_t("Asp Mk II", // className
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
                         true,        // hasStandardHyperdrive
                         true,        // hasStandardShields
                         true         // includesPulseLaser
    );

    // Register Sidewinder
    register_ship_type_t("Sidewinder", // className
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
                         true,         // hasStandardHyperdrive
                         true,         // hasStandardShields
                         true          // includesPulseLaser
    );

    // Register Python
    register_ship_type_t("Python", // className
                         200,      // baseHullStrength
                         80.0,     // baseShieldStrengthFront
                         80.0,     // baseShieldStrengthAft
                         7.0,      // maxFuelLY
                         3.0,      // fuelConsumptionRate (liters per 0.1 LY)
                         100,      // baseCargoCapacityTons
                         4,        // initialMissilePylons
                         50000.0,  // baseCost
                         20,       // baseSpeed
                         2,        // baseManeuverability
                         4,        // defaultWeaponSlots
                         2,        // defaultDefensiveSlots
                         3,        // defaultUtilitySlots
                         true,     // hasStandardHyperdrive
                         true,     // hasStandardShields
                         true      // includesPulseLaser
    );

    // Register Anaconda
    register_ship_type_t("Anaconda", // className
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
                         true,       // hasStandardHyperdrive
                         true,       // hasStandardShields
                         true        // includesPulseLaser
    );

    // Register Fer-de-Lance
    register_ship_type_t("Fer-de-Lance", // className
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
                         true,           // hasStandardHyperdrive
                         true,           // hasStandardShields
                         true            // includesPulseLaser
    );
}

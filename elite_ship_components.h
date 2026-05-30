#pragma once

#include <stddef.h>  // For NULL
#include <stdint.h>  // For uint16_t, int32_t
#include <stdbool.h> // For bool
#include <string.h>  // For string functions

// --- Constants for Equipment Types ---
#ifndef MAX_SHIP_NAME_LENGTH
#define MAX_SHIP_NAME_LENGTH 64
#endif

#ifndef MAX_EQUIPMENT_SLOTS
#define MAX_EQUIPMENT_SLOTS 10 // Max possible equipment slots
#endif

#ifndef MAX_CARGO_SLOTS
#define MAX_CARGO_SLOTS 50     // Max types of cargo a ship can hold
#endif

#ifndef MAX_EQUIPMENT_INVENTORY
#define MAX_EQUIPMENT_INVENTORY 30
#endif

// Backward compatibility constants for Cobra Mk III
#define COBRA_MK3_BASE_HULL_STRENGTH 100
#define COBRA_MK3_BASE_SHIELD_STRENGTH 50.0
#define COBRA_MK3_MAX_FUEL_LY 7.0
#define COBRA_MK3_BASE_CARGO_CAPACITY_TONS 20

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
} ShipCoreAttributes;

typedef struct ShipEquipmentItem
{
    char name[MAX_SHIP_NAME_LENGTH];
    EquipmentSlotType slotType;          // What kind of slot this is (e.g. Forward Weapon)
    int isActive;                        // 0 for empty/damaged, 1 for active
    EquipmentTypeSpecifics typeSpecific; // Use the new named union

    // Common attributes
    double damageOutput; // For weapons
} ShipEquipmentItem;

typedef struct CargoItem
{
    char name[MAX_SHIP_NAME_LENGTH]; // Name of the commodity
    int quantity;                    // Number of units
    int purchasePrice;               // Price per unit when bought (for player reference)
} CargoItem;

// --- Helper Functions (Names from types) ---

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

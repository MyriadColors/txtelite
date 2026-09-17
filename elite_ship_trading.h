#pragma once

#include "elite_navigation_types.h"
#include "elite_player_ship.h"
#include "elite_ship_components.h"
#include "elite_ship_inventory.h"
#include "elite_ship_registry.h"
#include "elite_state.h"
#include "platform_compat.h"
#include <ctype.h> // For isdigit()
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h> // For printf()
#include <stdlib.h>
#include <string.h>

// --- Constants for Ship Trading ---
#define MAX_SHIPS_AT_SHIPYARD 5       // Maximum number of ships available at a shipyard
#define SHIP_DEPRECIATION_RATE 0.05   // 5% depreciation per game month
#define MIN_TRADE_IN_VALUE_PERCENT 40 // Ship won't go below 40% of its new value

// --- Ship Availability by System Type ---
typedef struct ship_availability_t {
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // Ship class name
    bool availableInIndustrialSystems;
    bool availableInAgriculturalSystems;
    bool availableInMilitarySystems;
    double priceMultiplier; // Price multiplier based on system type
} ShipAvailability;

// --- Temporary Ship Storage ---
typedef struct temp_ship_storage_t {
    player_ship_t ship;
    bool isActive; // Whether there's a ship in storage
} temp_ship_storage_t;

// Global temporary storage for ship during trade-in
static temp_ship_storage_t g_trade_in_storage = {.isActive = false};

// --- Ship Availability Database ---
static const ShipAvailability shipAvailabilityDB[] = {
    {"Sidewinder", true, true, true, 0.8},       // Small, inexpensive starter ship
    {"Adder", true, true, true, 0.9},            // Common light trader
    {"Cobra Mk I", true, true, true, 0.95},      // Older multipurpose ship
    {"Cobra Mk III", true, true, true, 1.0},     // Available everywhere at standard price
    {"Viper", false, false, true, 0.9},          // Available in military systems, slightly cheaper
    {"Gecko", false, true, true, 1.0},           // Light combat ship
    {"Krait", true, false, true, 1.05},          // Combat-oriented ship
    {"Mamba", true, false, true, 1.1},           // Fast combat ship
    {"Asp Mk II", true, false, true, 1.1},       // Available in industrial and military
    {"Fer-de-Lance", false, false, true, 1.35},  // Premium military fighter
    {"Python", true, false, true, 1.25},         // Large multipurpose trader
    {"Boa", true, false, true, 1.3},             // Heavy transport ship
    {"Anaconda", true, false, true, 1.5},        // Largest and most expensive ship
    {"Moray Star Boat", true, true, false, 1.0}, // Civilian transport ship
    {"Transporter", true, true, false, 0.95}     // Commercial transport ship
};

#define NUM_SHIP_AVAILABILITY (sizeof(shipAvailabilityDB) / sizeof(ShipAvailability))

// --- Function Prototypes ---

/**
 * Determines if a ship type is available in the current system
 *
 * @param shipClassName Name of the ship class to check
 * @param systemEconomy Economy of the current system
 * @return 1 if ship is available, 0 otherwise
 */
static inline bool is_ship_available_in_system(const char *ship_class_name, int system_economy) {
    // Find the ship in the availability database
    for (size_t i = 0; i < NUM_SHIP_AVAILABILITY; i++) {
        if (strcmp(ship_class_name, shipAvailabilityDB[i].shipClassName) == 0) {
            // Check availability based on system economy
            switch (system_economy) {
            case 0: // Agricultural
                return shipAvailabilityDB[i].availableInAgriculturalSystems;
            case 1: // Industrial
                return shipAvailabilityDB[i].availableInIndustrialSystems;
            case 7: // Military
                return shipAvailabilityDB[i].availableInMilitarySystems;
            default:
                // For other economies, available if industrial or agricultural
                return (shipAvailabilityDB[i].availableInIndustrialSystems ||
                        shipAvailabilityDB[i].availableInAgriculturalSystems) != 0;
            }
        }
    }

    // If not found, assume not available
    return false;
}

/**
 * Gets the price multiplier for a ship in the current system
 *
 * @param shipClassName Name of the ship class
 * @param systemEconomy Economy of the current system
 * @return Price multiplier (1.0 is standard price)
 */
static inline double get_ship_price_multiplier(const char *ship_class_name, int system_economy) {
    // Find the ship in the availability database
    for (size_t i = 0; i < NUM_SHIP_AVAILABILITY; i++) {
        if (strcmp(ship_class_name, shipAvailabilityDB[i].shipClassName) == 0) {
            // Apply system-specific adjustments
            double base_multiplier = shipAvailabilityDB[i].priceMultiplier;

            // Additional adjustments based on system economy
            switch (system_economy) {
            case 0:                            // Agricultural
                return base_multiplier * 1.05; // 5% more expensive
            case 1:                            // Industrial
                return base_multiplier * 0.95; // 5% cheaper
            case 7:                            // Military
                return base_multiplier * 1.1;  // 10% more expensive
            default:
                return base_multiplier;
            }
        }
    }

    // If not found, assume standard price
    return 1.0;
}

/**
 * Gets a list of ships available for purchase in the current system
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param availableShips Array to store available ship types (must be at least MAX_SHIPS_AT_SHIPYARD)
 * @param shipPrices Array to store ship prices (must be at least MAX_SHIPS_AT_SHIPYARD)
 * @return Number of ships available
 */
static inline int get_available_ships(const char *system_name, int system_economy, const ship_type_t **available_ships,
                                      double *ship_prices) {
    // Initialize ship registry if needed
    intialize_ship_registry_t();

    int ship_count = 0;

    // Iterate through all registered ship types
    for (int i = 0; i < g_ship_registry.registeredShipCount && ship_count < MAX_SHIPS_AT_SHIPYARD; i++) {
        const ship_type_t *ship_type = &g_ship_registry.ship_type_ts[i];

        // Check if ship is available in this system
        if (is_ship_available_in_system(ship_type->className, system_economy)) {
            // Add to the available ships list
            available_ships[ship_count] = ship_type;

            // Calculate price with system-specific multiplier
            double multiplier = get_ship_price_multiplier(ship_type->className, system_economy);
            ship_prices[ship_count] = ship_type->baseCost * multiplier;

            ship_count++;
        }
    }

    // Log availability for debugging (uses systemName parameter)
    if (ship_count == 0 && system_name != nullptr) {
        printf("Debug: No ships available at %s shipyard (economy type: %d)\n", system_name, system_economy);
    }

    return ship_count;
}

/**
 * Calculates the trade-in value of the player's current ship
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param gameTime Current game time in seconds
 * @return Trade-in value in credits
 */
static inline double calculate_trade_in_value(const player_ship_t *player_ship, uint64_t game_time) {
    if (player_ship == nullptr || player_ship->ship_type_t == nullptr) {
        return 0.0;
    }

    // Start with base cost of the ship
    double base_value = player_ship->ship_type_t->baseCost;

    // Apply condition-based depreciation
    int hull_percentage = (player_ship->attributes.hullStrength * 100) / player_ship->ship_type_t->baseHullStrength;
    double condition_factor = (double)hull_percentage / 100.0;

    // Apply time-based depreciation (for future expansion)
    // Currently we don't track the ship purchase time, so this is simplified
    double game_months = (double)game_time / (30.0 * 24.0 * 60.0 * 60.0); // Rough estimate of game months
    double time_factor = 1.0 - (game_months * SHIP_DEPRECIATION_RATE);
    if (time_factor < (MIN_TRADE_IN_VALUE_PERCENT / 100.0)) {
        time_factor = (MIN_TRADE_IN_VALUE_PERCENT / 100.0);
    }

    // Calculate value of installed equipment (not including standard equipment)
    double equipment_value = 0.0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (player_ship->equipment[i].isActive) {
            // For simplicity, we're assuming a fixed value per equipment item
            // This could be expanded to actual equipment costs
            equipment_value += 200.0; // Arbitrary value per equipment item
        }
    }

    // Calculate trade-in value
    double trade_in_value = (base_value * condition_factor * time_factor) + (equipment_value * 0.75);

    return trade_in_value;
}

/**
 * Displays a list of ships available for purchase
 *
 * @param system_name Name of the current star system
 * @param system_economy Economy of the current system
 * @param player_ship Pointer to the player_ship_tstructure
 * @param game_time Current game time in seconds
 */
static inline void display_shipyard(const char *system_name, int system_economy, const player_ship_t *player_ship,
                                    uint64_t game_time) {
    // Get list of available ships
    const ship_type_t *available_ships[MAX_SHIPS_AT_SHIPYARD];
    double ship_prices[MAX_SHIPS_AT_SHIPYARD];
    int ship_count = get_available_ships(system_name, system_economy, available_ships, ship_prices);

    // Calculate trade-in value of current ship
    double trade_in_value = calculate_trade_in_value(player_ship, game_time);

    // Display shipyard header
    printf("\n=== %s Shipyard ===\n", system_name);
    printf("Your current ship: %s (%s)\n", player_ship->shipName, player_ship->shipClassName);
    printf("Trade-in value: %.1f CR\n\n", trade_in_value);
    // Display available ships
    printf("Available Ships:\n");
    printf("%-4s %-15s %-8s %-6s %-7s %-8s %-10s\n", "ID", "Ship Class", "Hull", "Cargo", "Cost", "Net Cost", "Status");
    printf("%-4s %-15s %-8s %-6s %-7s %-8s %-10s\n", "--", "----------", "----", "-----", "----", "--------", "------");

    for (int i = 0; i < ship_count; i++) {
        const ship_type_t *ship = available_ships[i];
        double price = ship_prices[i];
        double net_cost = price - trade_in_value;

        // Determine if the player can afford this ship
        // Cash is stored internally as a value 10x the displayed value
        bool can_afford = (net_cost * 10.0 <= g_state.Cash);
        printf("[%d] %-15s %-8d %-6d %-7.1f %-8.1f %s\n",
               i + 1, // Use 1-based indexing for user-friendliness
               ship->className, ship->baseHullStrength, ship->baseCargoCapacityTons, price, net_cost,
               (int)can_afford ? "AVAILABLE" : "TOO EXPENSIVE");
    }

    printf("\nUse 'buyship <ID>' to purchase a new ship (e.g., 'buyship 1').\n");
    printf("Use 'buyship <ID> notrade' to buy without trading in your current ship.\n");
    printf("Use 'compareship <shipname>' to compare with your current ship.\n");
}

/**
 * Compares the player's current ship with a ship available for purchase
 *
 * @param player_ship Pointer to the player_ship_tstructure
 * @param compare_ship_name Name of the ship to compare with
 */
static inline void compare_ships(const player_ship_t *player_ship, const char *compare_ship_name) {
    if (player_ship == nullptr || compare_ship_name == nullptr) {
        printf("Error: Invalid ship data.\n");
        return;
    }

    // Find the ship type to compare with
    intialize_ship_registry_t();
    const ship_type_t *compare_ship = get_ship_type_t_by_name(compare_ship_name);

    if (compare_ship == nullptr) {
        printf("Error: Ship '%s' not found.\n", compare_ship_name);
        return;
    }

    // Display comparison
    printf("\n=== Ship Comparison: %s vs %s ===\n", player_ship->shipClassName, compare_ship->className);

    printf("%-20s %-15s %-15s %-15s\n", "Specification", player_ship->shipClassName, compare_ship->className,
           "Difference");
    printf("%-20s %-15s %-15s %-15s\n", "-------------", "---------------", "---------------", "----------");

    // Compare hull strength
    printf("%-20s %-15d %-15d %+d\n", "Hull Strength", player_ship->ship_type_t->baseHullStrength,
           compare_ship->baseHullStrength,
           compare_ship->baseHullStrength - player_ship->ship_type_t->baseHullStrength); // Compare shield strength
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Shield (Front)", player_ship->ship_type_t->baseShieldStrengthFront,
           compare_ship->baseShieldStrengthFront,
           compare_ship->baseShieldStrengthFront - player_ship->ship_type_t->baseShieldStrengthFront);
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Shield (Aft)", player_ship->ship_type_t->baseShieldStrengthAft,
           compare_ship->baseShieldStrengthAft,
           compare_ship->baseShieldStrengthAft - player_ship->ship_type_t->baseShieldStrengthAft);

    // Compare fuel capacity
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Fuel Capacity (LY)", player_ship->ship_type_t->maxFuelLY,
           compare_ship->maxFuelLY, compare_ship->maxFuelLY - player_ship->ship_type_t->maxFuelLY);

    // Compare cargo capacity
    printf("%-20s %-15d %-15d %+d\n", "Cargo Capacity (T)", player_ship->ship_type_t->baseCargoCapacityTons,
           compare_ship->baseCargoCapacityTons,
           compare_ship->baseCargoCapacityTons - player_ship->ship_type_t->baseCargoCapacityTons);

    // Compare missile pylons
    printf("%-20s %-15d %-15d %+d\n", "Missile Pylons", player_ship->ship_type_t->initialMissilePylons,
           compare_ship->initialMissilePylons,
           compare_ship->initialMissilePylons - player_ship->ship_type_t->initialMissilePylons);

    // Compare speed
    printf("%-20s %-15d %-15d %+d\n", "Speed", player_ship->ship_type_t->baseSpeed, compare_ship->baseSpeed,
           compare_ship->baseSpeed - player_ship->ship_type_t->baseSpeed);

    // Compare maneuverability
    printf("%-20s %-15d %-15d %+d\n", "Maneuverability", player_ship->ship_type_t->baseManeuverability,
           compare_ship->baseManeuverability,
           compare_ship->baseManeuverability - player_ship->ship_type_t->baseManeuverability);

    // Compare equipment slots
    printf("%-20s %-15d %-15d %+d\n", "Weapon Slots", player_ship->ship_type_t->defaultWeaponSlots,
           compare_ship->defaultWeaponSlots,
           compare_ship->defaultWeaponSlots - player_ship->ship_type_t->defaultWeaponSlots);
    printf("%-20s %-15d %-15d %+d\n", "Defensive Slots", player_ship->ship_type_t->defaultDefensiveSlots,
           compare_ship->defaultDefensiveSlots,
           compare_ship->defaultDefensiveSlots - player_ship->ship_type_t->defaultDefensiveSlots);
    printf("%-20s %-15d %-15d %+d\n", "Utility Slots", player_ship->ship_type_t->defaultUtilitySlots,
           compare_ship->defaultUtilitySlots,
           compare_ship->defaultUtilitySlots - player_ship->ship_type_t->defaultUtilitySlots); // Compare cost
    printf("%-20s %-15.1f %-15.1f %+.1f\n", "Base Cost (CR)", player_ship->ship_type_t->baseCost,
           compare_ship->baseCost, compare_ship->baseCost - player_ship->ship_type_t->baseCost);
}

/**
 * Transfer equipment from one ship to another
 *
 * @param source_ship Source ship to transfer from
 * @param target_ship Target ship to transfer to
 * @return Number of equipment items transferred
 */
static inline int transfer_equipment(player_ship_t *source_ship, player_ship_t *target_ship) {
    if (source_ship == nullptr || target_ship == nullptr) {
        return 0;
    }

    int transfer_count = 0;

    // Check each equipment slot in the source ship
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (source_ship->equipment[i].isActive) {
            // Check if the target ship has this slot type
            equipment_slot_type_t slot_type = source_ship->equipment[i].slotType;

            // Skip standard pulse laser if the target ship already includes one
            if (slot_type == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON &&
                source_ship->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_PULSE_LASER &&
                target_ship->ship_type_t->includesPulseLaser) {
                continue;
            }

            // Check if slot is valid for the target ship
            bool valid_slot = false;
            switch (slot_type) {
            case EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON:
            case EQUIPMENT_SLOT_TYPE_AFT_WEAPON:
                valid_slot = (i < target_ship->ship_type_t->defaultWeaponSlots);
                break;

            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_1:
            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_2:
                valid_slot = (i < target_ship->ship_type_t->defaultDefensiveSlots);
                break;

            case UTILITY_SYSTEM_1:
            case UTILITY_SYSTEM_2:
            case UTILITY_SYSTEM_3:
            case UTILITY_SYSTEM_4:
                valid_slot = (i < target_ship->ship_type_t->defaultUtilitySlots);
                break;

            default:
                valid_slot = false;
                break;
            }

            if (valid_slot && !target_ship->equipment[i].isActive) {
                // Copy equipment from source to target
                target_ship->equipment[i] = source_ship->equipment[i];
                transfer_count++;

                // Clear the slot in the source ship
                source_ship->equipment[i].isActive = false;
                if (safe_snprintf(source_ship->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
                    source_ship->equipment[i].name[0] = '\0';
                }
            } else {
                // Store in inventory if slot not available
                bool stored = false;
                for (int j = 0; j < MAX_EQUIPMENT_INVENTORY; j++) {
                    if (!target_ship->equipmentInventory[j].isActive) {
                        target_ship->equipmentInventory[j] = source_ship->equipment[i];
                        stored = true;
                        transfer_count++;
                        break;
                    }
                }

                if (stored) {
                    // Clear the slot in the source ship
                    source_ship->equipment[i].isActive = false;
                    if (safe_snprintf(source_ship->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
                        source_ship->equipment[i].name[0] = '\0';
                    }
                } else {
                    printf("Warning: Could not transfer %s - no free slots or inventory space.\n",
                           source_ship->equipment[i].name);
                }
            }
        }
    }

    // Transfer inventory items
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; i++) {
        if (source_ship->equipmentInventory[i].isActive) {
            bool stored = false;
            for (int j = 0; j < MAX_EQUIPMENT_INVENTORY; j++) {
                if (!target_ship->equipmentInventory[j].isActive) {
                    target_ship->equipmentInventory[j] = source_ship->equipmentInventory[i];
                    stored = true;
                    transfer_count++;
                    break;
                }
            }

            if (stored) {
                // Clear the slot in the source ship
                source_ship->equipmentInventory[i].isActive = false;
                if (safe_snprintf(source_ship->equipmentInventory[i].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
                    source_ship->equipmentInventory[i].name[0] = '\0';
                }
            } else {
                printf("Warning: Could not transfer inventory item %s - no free inventory space.\n",
                       source_ship->equipmentInventory[i].name);
            }
        }
    }

    return transfer_count;
}

/**
 * Transfer cargo from one ship to another
 *
 * @param source_ship Source ship to transfer from
 * @param target_ship Target ship to transfer to
 * @return Amount of cargo (in tons) that could not be transferred due to space limitations
 */
static inline int transfer_cargo(player_ship_t *source_ship, player_ship_t *target_ship) {
    if (source_ship == nullptr || target_ship == nullptr) {
        return 0;
    }

    int unable_to_transfer = 0;

    // Check each cargo slot in the source ship
    for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
        if (source_ship->cargo[i].quantity > 0) {
            // Check if we have enough space in the target ship
            int available_space = target_ship->attributes.cargoCapacityTons - target_ship->attributes.currentCargoTons;

            if (available_space >= source_ship->cargo[i].quantity) {
                // We have enough space to transfer all
                // Look for the same cargo type in the target ship
                int target_slot = -1;
                for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                    if (target_ship->cargo[j].quantity > 0 &&
                        strcmp(target_ship->cargo[j].name, source_ship->cargo[i].name) == 0) {
                        target_slot = j;
                        break;
                    }
                }

                // If not found, find an empty slot
                if (target_slot == -1) {
                    for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                        if (target_ship->cargo[j].quantity == 0) {
                            target_slot = j;
                            break;
                        }
                    }
                }

                // If we found a slot, transfer the cargo
                if (target_slot != -1) {
                    // If it's an empty slot, copy the cargo details
                    if (target_ship->cargo[target_slot].quantity == 0) {
                        // Copy cargo details
                        target_ship->cargo[target_slot].quantity = source_ship->cargo[i].quantity;
                        target_ship->cargo[target_slot].purchasePrice = source_ship->cargo[i].purchasePrice;
                        int name_length = safe_snprintf(target_ship->cargo[target_slot].name, MAX_SHIP_NAME_LENGTH,
                                                        "%s", source_ship->cargo[i].name);
                        if (name_length < 0) {
                            target_ship->cargo[target_slot].name[0] = '\0';
                        } else if (name_length >= MAX_SHIP_NAME_LENGTH) {
                            target_ship->cargo[target_slot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
                        }
                        target_ship->cargo[target_slot].name[MAX_SHIP_NAME_LENGTH - 1] =
                            '\0'; // Ensure nullptr termination
                    }

                    // Update quantities
                    target_ship->cargo[target_slot].quantity += source_ship->cargo[i].quantity;
                    target_ship->attributes.currentCargoTons += source_ship->cargo[i].quantity;
                    source_ship->attributes.currentCargoTons -= source_ship->cargo[i].quantity;
                    source_ship->cargo[i].quantity = 0;
                } else {
                    // No free slots in target ship, shouldn't happen with MAX_CARGO_SLOTS
                    unable_to_transfer += source_ship->cargo[i].quantity;
                    printf("Warning: Could not transfer %d tons of %s - no free cargo slots.\n",
                           source_ship->cargo[i].quantity, source_ship->cargo[i].name);
                }
            } else if (available_space > 0) {
                // We can transfer some but not all
                // Look for the same cargo type in the target ship
                int target_slot = -1;
                for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                    if (target_ship->cargo[j].quantity > 0 &&
                        strcmp(target_ship->cargo[j].name, source_ship->cargo[i].name) == 0) {
                        target_slot = j;
                        break;
                    }
                }

                // If not found, find an empty slot
                if (target_slot == -1) {
                    for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                        if (target_ship->cargo[j].quantity == 0) {
                            target_slot = j;
                            break;
                        }
                    }
                }

                // If we found a slot, transfer as much cargo as possible
                if (target_slot != -1) {
                    // If it's an empty slot, copy the cargo details
                    if (target_ship->cargo[target_slot].quantity == 0) {
                        // Copy cargo details
                        target_ship->cargo[target_slot].quantity = source_ship->cargo[i].quantity;
                        target_ship->cargo[target_slot].purchasePrice = source_ship->cargo[i].purchasePrice;
                        int name_length = safe_snprintf(target_ship->cargo[target_slot].name, MAX_SHIP_NAME_LENGTH,
                                                        "%s", source_ship->cargo[i].name);
                        if (name_length < 0) {
                            target_ship->cargo[target_slot].name[0] = '\0';
                        }
                        target_ship->cargo[target_slot].name[MAX_SHIP_NAME_LENGTH - 1] =
                            '\0'; // Ensure nullptr termination
                    }

                    // Update quantities
                    int amount_to_transfer = available_space;
                    target_ship->cargo[target_slot].quantity += amount_to_transfer;
                    target_ship->attributes.currentCargoTons += amount_to_transfer;
                    source_ship->attributes.currentCargoTons -= amount_to_transfer;
                    source_ship->cargo[i].quantity -= amount_to_transfer;

                    unable_to_transfer += source_ship->cargo[i].quantity;
                    printf("Warning: Only transferred %d of %d tons of %s due to space limitations.\n",
                           amount_to_transfer, amount_to_transfer + source_ship->cargo[i].quantity,
                           source_ship->cargo[i].name);
                } else {
                    // No free slots in target ship, shouldn't happen with MAX_CARGO_SLOTS
                    unable_to_transfer += source_ship->cargo[i].quantity;
                    printf("Warning: Could not transfer %d tons of %s - no free cargo slots.\n",
                           source_ship->cargo[i].quantity, source_ship->cargo[i].name);
                }
            } else {
                // No space at all in target ship
                unable_to_transfer += source_ship->cargo[i].quantity;
                printf("Warning: Could not transfer %d tons of %s - no cargo space available.\n",
                       source_ship->cargo[i].quantity, source_ship->cargo[i].name);
            }
        }
    }

    return unable_to_transfer;
}

/**
 * Buy a new ship, optionally trading in the current ship
 *
 * @param systemName Name of the current star system
 * @param systemEconomy Economy of the current system
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param newShipName Name of the ship to buy
 * @param gameTime Current game time in seconds
 * @param tradeIn Whether to trade in the current ship
 * @return 1 if purchase successful, 0 otherwise
 */
static inline bool initialize_purchased_ship(player_ship_t *player_ship, const ship_type_t *ship_type, bool trade_in) {
    if (trade_in) {
        g_trade_in_storage.ship = *player_ship;
        g_trade_in_storage.isActive = true;
    }

    memset(player_ship, 0, sizeof(player_ship_t));
    if (initialize_ship(player_ship, ship_type, nullptr)) {
        return true;
    }

    if (trade_in) {
        *player_ship = g_trade_in_storage.ship;
        g_trade_in_storage.isActive = false;
    }
    return false;
}

static inline void display_purchase_summary(const ship_type_t *ship_type, double price, double trade_in_value,
                                            double net_cost, int equipment_transferred, int cargo_lost, bool trade_in) {
    printf("\nCongratulations on your new ship purchase!\n");
    printf("You are now the proud owner of a %s.\n", ship_type->className);
    printf("Purchase price: %.1f CR\n", price);
    if (trade_in) {
        printf("Trade-in value: %.1f CR\n", trade_in_value);
        printf("Equipment transferred: %d items\n", equipment_transferred);
        if (cargo_lost > 0) {
            printf("Warning: %d tons of cargo could not be transferred due to space limitations.\n", cargo_lost);
        }
    }
    printf("Net cost: %.1f CR\n", net_cost);
    printf("Remaining cash: %.1f CR\n", (double)g_state.Cash / 10.0);
}

static inline bool buy_new_ship(const char *system_name, int system_economy, player_ship_t *player_ship,
                                const char *new_ship_name, uint64_t game_time, bool trade_in) {
    (void)system_name; // Unused parameter
    if (player_ship == nullptr || new_ship_name == nullptr) {
        printf("Error: Invalid ship data.\n");
        return false;
    }

    // Find the new ship type
    intialize_ship_registry_t();
    const ship_type_t *newship_type_t = get_ship_type_t_by_name(new_ship_name);
    if (newship_type_t == nullptr) {
        printf("Error: Ship '%s' not found.\n", new_ship_name);
        return false;
    } // Check if the ship is available in this system
    if (!is_ship_available_in_system(newship_type_t->className, system_economy)) {
        printf("Error: Ship '%s' is not available in this star system.\n", newship_type_t->className);
        return false;
    }

    // Calculate the price with system-specific multiplier
    double multiplier = get_ship_price_multiplier(newship_type_t->className, system_economy);
    double price = newship_type_t->baseCost * multiplier;

    // Calculate trade-in value of current ship if trading in
    double trade_in_value = 0.0;
    if (trade_in) {
        trade_in_value = calculate_trade_in_value(player_ship, game_time);
    } // Calculate net cost
    double net_cost = price - trade_in_value;
    // Check if player can afford the ship
    // Cash is stored internally as a value 10x the displayed value
    if (net_cost * 10.0 > g_state.Cash) {
        printf("Error: Insufficient funds to purchase %s.\n", newship_type_t->className);
        printf("Ship price: %.1f CR, Trade-in value: %.1f CR, Net cost: %.1f CR, Your cash: %.1f CR\n", price,
               trade_in_value, net_cost, (double)g_state.Cash / 10.0);
        return false;
    }

    // If trading in, store the current ship temporarily
    if (!initialize_purchased_ship(player_ship, newship_type_t, trade_in)) {
        printf("Error: Failed to initialize new ship.\n");
        return false;
    }
    // Transfer equipment if trading in
    int equipment_transferred = 0;
    int cargo_lost = 0;
    if (trade_in) {
        equipment_transferred = transfer_equipment(&g_trade_in_storage.ship, player_ship);
        cargo_lost = transfer_cargo(&g_trade_in_storage.ship, player_ship);

        // Clear the trade-in storage
        g_trade_in_storage.isActive = false;
    }
    // Deduct the cost from player's cash
    g_state.Cash -= (int32_t)(net_cost * 10.0);

    // Display purchase information
    display_purchase_summary(newship_type_t, price, trade_in_value, net_cost, equipment_transferred, cargo_lost,
                             trade_in);

    return true;
}

// Command handler functions to be included in elite_commands.h

/**
 * Command handler for the 'shipyard' command
 * Shows ships available for purchase at the current station
 *
 * @param arguments Command arguments (unused)
 * @return 1 if command handled successfully
 */
[[maybe_unused]] static inline bool shipyard_command(const char *arguments) {
    (void)arguments; // Unused parameter
    // Check if player is docked at a station
    // Need to be both at a station AND docked
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION || g_state.PlayerLocationType != 10) {
        printf("Error: You must be docked at a station to access the shipyard.\n");
        return false;
    }

    // Display the shipyard
    display_shipyard(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, g_state.PlayerShipPtr,
                     g_state.currentGameTimeSeconds);

    return true;
}

/**
 * Command handler for the 'compare' command
 * Compares the player's current ship with a ship available for purchase
 *
 * @param arguments Name of the ship to compare with
 * @return 1 if command handled successfully
 */
[[maybe_unused]] static inline bool compare_ship_command(const char *arguments) {
    // Check if arguments are provided
    if (arguments == nullptr || arguments[0] == '\0') {
        printf("Error: Please specify a ship to compare with.\n");
        printf("Usage: compare <shipname>\n");
        return false;
    }

    // Compare ships
    compare_ships(g_state.PlayerShipPtr, arguments);

    return true;
}

/**
 * Gets the name of a ship based on its shipyard ID
 *
 * @param system_name Name of the current star system
 * @param system_economy Economy of the current system
 * @param ship_id ID of the ship (1-based index as shown in the shipyard)
 * @param ship_name Buffer to store the ship name (must be pre-allocated)
 * @param ship_name_size Size of the shipName buffer
 * @return 1 if the ship was found, 0 otherwise
 */
static inline bool get_ship_name_by_id(const char *system_name, int system_economy, int ship_id, char *ship_name,
                                       size_t ship_name_size) {
    if (ship_id < 1 || ship_name == nullptr || ship_name_size < 1) {
        return false;
    }

    // Get list of available ships
    const ship_type_t *available_ships[MAX_SHIPS_AT_SHIPYARD];
    double ship_prices[MAX_SHIPS_AT_SHIPYARD];
    int ship_count = get_available_ships(system_name, system_economy, available_ships, ship_prices);

    // Adjust shipID to 0-based index
    int ship_index = ship_id - 1;

    // Check if the shipID is valid
    if (ship_index < 0 || ship_index >= ship_count) {
        return false;
    }

    // Copy the ship name to the buffer
    int name_length = safe_snprintf(ship_name, ship_name_size, "%s", available_ships[ship_index]->className);
    return (name_length < 0 || (size_t)(name_length) >= ship_name_size) != 0;
}

/**
 * Command handler for the 'buyship' command
 * Buys a new ship, optionally trading in the current ship
 *
 * @param arguments Ship ID or name to buy, with optional 'notrade' flag
 * @return 1 if command handled successfully
 */
[[maybe_unused]] static inline bool buy_ship_command(const char *arguments) {
    // Check if player is docked at a station
    // Need to be both at a station AND docked
    if (g_state.PlayerNavState.currentLocationType != CELESTIAL_STATION || g_state.PlayerLocationType != 10) {
        printf("Error: You must be docked at a station to purchase a ship.\n");
        return false;
    }

    // Check if arguments are provided
    if (arguments == nullptr || arguments[0] == '\0') {
        printf("Error: Please specify a ship to buy.\n");
        printf("Usage: buyship <ID or shipname> [notrade]\n");
        printf("Example: buyship 1  or  buyship \"Cobra Mk III\"\n");
        return false;
    }

    // Parse arguments
    char ship_name_or_id[64] = {0};
    bool trade_in = true;

    // Copy the first part of the arguments (up to the first space)
    const char *space = strchr(arguments, ' ');
    if (space != nullptr) {
        size_t name_len = (size_t)(space - arguments);
        const size_t COPY_LEN = name_len < sizeof(ship_name_or_id) - 1 ? name_len : sizeof(ship_name_or_id) - 1;
        memcpy(ship_name_or_id, arguments, COPY_LEN);
        ship_name_or_id[COPY_LEN] = '\0';

        // Check for 'notrade' flag in the remaining part
        if (strstr(space + 1, "notrade") != nullptr) {
            trade_in = false;
        }
    } else {
        // No space, just copy the entire argument
        const size_t NAME_LEN = strlen(arguments);
        const size_t COPY_LEN = NAME_LEN < sizeof(ship_name_or_id) - 1 ? NAME_LEN : sizeof(ship_name_or_id) - 1;
        memcpy(ship_name_or_id, arguments, COPY_LEN);
        ship_name_or_id[COPY_LEN] = '\0';
    }

    // Check if the argument is a number (ID) or a string (ship name)
    char actual_ship_name[MAX_SHIP_NAME_LENGTH] = {0};
    bool is_id = true;

    // Check if shipNameOrID is a number
    for (size_t i = 0; i < strlen(ship_name_or_id); i++) {
        if (!isdigit(ship_name_or_id[i])) {
            is_id = false;
            break;
        }
    }

    if (is_id) {
        // Convert the ID to an integer
        char *endptr = NULL;
        errno = 0;
        long parsed_ship_id = strtol(ship_name_or_id, &endptr, 10);
        if (ship_name_or_id[0] == '\0' || *endptr != '\0' || errno == ERANGE || parsed_ship_id < INT_MIN ||
            parsed_ship_id > INT_MAX) {
            printf("Error: Invalid ship ID: %s\n", ship_name_or_id);
            return false;
        }
        int ship_id = (int)parsed_ship_id;

        // Get the ship name by ID
        if (!get_ship_name_by_id(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, ship_id, actual_ship_name,
                                 MAX_SHIP_NAME_LENGTH)) {
            printf("Error: Invalid ship ID: %d\n", ship_id);
            return false;
        }
    } else {
        // The argument is a ship name, just copy it
        int name_length = safe_snprintf(actual_ship_name, sizeof(actual_ship_name), "%.63s", ship_name_or_id);
        if (name_length < 0 || (size_t)name_length >= sizeof(actual_ship_name)) {
            printf("Error: Invalid ship name.\n");
            return false;
        }
        actual_ship_name[MAX_SHIP_NAME_LENGTH - 1] = '\0'; // Ensure nullptr-termination
    }

    // Buy the new ship
    return buy_new_ship(g_state.CurrentSystemName, g_state.CurrentSystemEconomy, g_state.PlayerShipPtr,
                        actual_ship_name, g_state.currentGameTimeSeconds, trade_in);
}

#pragma once

#include "elite_ship_components.h"
#include "elite_ship_equipment.h"
#include "elite_ship_inventory.h"
#include "elite_ship_registry.h"
#include "elite_state.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct player_ship_t {
    char shipName[MAX_SHIP_NAME_LENGTH];
    char shipClassName[MAX_SHIP_NAME_LENGTH]; // e.g., "Cobra Mk III"
    const ship_type_t *ship_type_t;           // Pointer to the ship type definition
    ship_core_attributes_t attributes;
    ship_equipment_item_t equipment[MAX_EQUIPMENT_SLOTS];              // Currently equipped items
    ship_equipment_item_t equipmentInventory[MAX_EQUIPMENT_INVENTORY]; // Inventory of stored equipment
    cargo_item_t cargo[MAX_CARGO_SLOTS];
} player_ship_t;

// --- Ship Operations ---

/**
 * Initializes a player_ship_t with the given ship type
 *
 * @return 1 if successful, 0 otherwise
 */
static inline bool set_ship_text(char *destination, size_t destination_size, const char *source) {
    const int RESULT = safe_snprintf(destination, destination_size, "%s", source);
    return (RESULT >= 0 && (size_t)(RESULT) < destination_size) != 0;
}

static inline void initialize_empty_ship_contents(player_ship_t *player_ship_t) {
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        player_ship_t->equipment[i].isActive = false;
        (void)set_ship_text(player_ship_t->equipment[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        player_ship_t->equipment[i].typeSpecific.utilityType = UTILITY_SYSTEM_TYPE_NONE;
    }
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i) {
        player_ship_t->equipmentInventory[i].isActive = false;
        (void)set_ship_text(player_ship_t->equipmentInventory[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        player_ship_t->equipmentInventory[i].slotType = EQUIPMENT_SLOT_TYPE_NONE;
        player_ship_t->equipmentInventory[i].damageOutput = 0.0;
    }
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        (void)set_ship_text(player_ship_t->cargo[i].name, MAX_SHIP_NAME_LENGTH, "Empty");
        player_ship_t->cargo[i].quantity = 0;
    }
}

static inline bool initialize_ship(player_ship_t *player_ship_t, const ship_type_t *ship_type_t,
                                   const char *custom_name) {
    if (player_ship_t == nullptr || ship_type_t == nullptr) {
        return false;
    }

    // Set ship name (custom or default)
    if (custom_name != nullptr && custom_name[0] != '\0') {
        if (!set_ship_text(player_ship_t->shipName, MAX_SHIP_NAME_LENGTH, custom_name)) {
            return false;
        }
    } else {
        // Construct default name if no custom name is provided
        char default_name[MAX_SHIP_NAME_LENGTH];
        const int RESULT = safe_snprintf(default_name, sizeof(default_name), "%s Class", ship_type_t->className);
        if (RESULT < 0 || (size_t)(RESULT) >= sizeof(default_name) ||
            !set_ship_text(player_ship_t->shipName, MAX_SHIP_NAME_LENGTH, default_name)) {
            return false;
        }
    }
    if (!set_ship_text(player_ship_t->shipClassName, MAX_SHIP_NAME_LENGTH, ship_type_t->className)) {
        return false;
    }

    // Set the ship type pointer
    player_ship_t->ship_type_t = ship_type_t;

    // Initialize core attributes based on ship type
    player_ship_t->attributes.hullStrength = ship_type_t->baseHullStrength;
    player_ship_t->attributes.shieldStrengthFront = ship_type_t->baseShieldStrengthFront;
    player_ship_t->attributes.shieldStrengthAft = ship_type_t->baseShieldStrengthAft;
    player_ship_t->attributes.fuelLiters = ship_type_t->maxFuelLY * 100.0; // Assuming 100 liters per LY
    player_ship_t->attributes.cargoCapacityTons = ship_type_t->baseCargoCapacityTons;
    player_ship_t->attributes.currentCargoTons = 0;
    player_ship_t->attributes.missilePylons = ship_type_t->initialMissilePylons;
    player_ship_t->attributes.missilesLoadedHoming = 0;
    player_ship_t->attributes.missilesLoadedDumbfire = 0;

    initialize_empty_ship_contents(player_ship_t);

    // Add pulse laser if the ship type includes one
    if (ship_type_t->includesPulseLaser) {
        player_ship_t->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].isActive = true;
        if (!set_ship_text(player_ship_t->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].name, MAX_SHIP_NAME_LENGTH,
                           "Pulse Laser")) {
            return false;
        }
        player_ship_t->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
        player_ship_t->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].typeSpecific.weaponType = WEAPON_TYPE_PULSE_LASER;
        player_ship_t->equipment[EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON].damageOutput = 5.0; // Example
    }

    return true;
}

/**
 * Initialize the ship registry and then initialize a player_ship_t to Cobra Mk III default specifications.
 */
[[maybe_unused]] static inline void initialize_cobra_mk_iii(player_ship_t *player_ship_t) {
    if (player_ship_t == nullptr) {
        return;
    }

    // Ensure the ship registry is initialized
    intialize_ship_registry_t();

    // Get the Cobra Mk III ship type from the registry
    const ship_type_t *cobra_mk_iii = get_ship_type_t_by_name("Cobra Mk III");
    if (cobra_mk_iii == nullptr) {
        printf("Error: Could not find Cobra Mk III ship type in registry.\n");
        return;
    }

    initialize_ship(player_ship_t, cobra_mk_iii, nullptr);
}

/**
 * Displays the current status of the player's ship.
 */
[[maybe_unused]] static inline void display_ship_status(const player_ship_t *player_ship) {
    bool ecm_found = false;
    bool escape_pod_found = false;
    bool fuel_scoops_found = false;
    bool docking_computer_found = false;
    bool scanner_upgrade_found = false;
    bool rear_laser_found = false;
    bool forward_pulse_laser_found = false;

    if (player_ship == nullptr) {
        printf("Error: Ship data is nullptr.\n");
        return;
    }

    printf("\n--- %s (%s) Status ---\n", player_ship->shipName, player_ship->shipClassName);
    printf("Hull Strength: %d / %d\n", player_ship->attributes.hullStrength,
           player_ship->ship_type_t->baseHullStrength);
    printf("Shields (F/A): %.2f / %.2f\n", player_ship->attributes.shieldStrengthFront,
           player_ship->attributes.shieldStrengthAft);
    // Convert Liters to LY for display, assuming 1 LY = 100 Liters (example factor)
    printf("Fuel: %.2f LY (%.0F Liters)\n", player_ship->attributes.fuelLiters / 100.0,
           player_ship->attributes.fuelLiters);
    printf("Cargo: %dT / %dT\n", player_ship->attributes.currentCargoTons, player_ship->attributes.cargoCapacityTons);
    printf("Missile Pylons: %d (Homing: %d, Dumbfire: %d)\n", player_ship->attributes.missilePylons,
           player_ship->attributes.missilesLoadedHoming, player_ship->attributes.missilesLoadedDumbfire);

    int has_equipment = 0;
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive && strlen(player_ship->equipment[i].name) > 0 &&
            strcmp(player_ship->equipment[i].name, "Empty") != 0) {
            has_equipment = 1;
            printf("- %s", player_ship->equipment[i].name);

            // Only print slot info if it's useful
            if (player_ship->equipment[i].slotType != EQUIPMENT_SLOT_TYPE_NONE) {
                printf(" (Slot: %d", player_ship->equipment[i].slotType);

                // Print the type info based on slot type
                if (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
                    player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
                    printf(", Type: Weapon - %d", player_ship->equipment[i].typeSpecific.weaponType);
                } else if (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                           player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
                    printf(", Type: Defensive - %d", player_ship->equipment[i].typeSpecific.defensiveType);
                } else if (player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                           player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4) {
                    printf(", Type: Utility - %d", player_ship->equipment[i].typeSpecific.utilityType);
                }
                printf(")");
            }
            printf("\n");
        }
    }

    if (!has_equipment) {
        printf("No active equipment.\n");
    }

    printf("\n--- Key Systems & Upgrades ---\n");

    bool is_cobra_mk_iii = (strcmp(player_ship->shipClassName, "Cobra Mk III") == 0);
    if (is_cobra_mk_iii) {
        printf("- Basic Shields System\n");
    }

    // Display fuel-related information for all ships
    printf("- %s Hyperspace Drive (%.1f LY Max, %.1f CR per 0.1 LY)\n",
           (int)player_ship->ship_type_t->hasStandardHyperdrive ? "Standard" : "Enhanced",
           player_ship->ship_type_t->maxFuelLY, player_ship->ship_type_t->fuelConsumptionRate / 10.0);

    // Standard Cargo Bay is reflected in attributes.cargoCapacityTons
    printf("- Standard Cargo Bay (%dT)\n", player_ship->ship_type_t->baseCargoCapacityTons);

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive) {
            // Forward Pulse Laser
            if (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON &&
                player_ship->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_PULSE_LASER) {
                if (!forward_pulse_laser_found) {
                    if (is_cobra_mk_iii) {
                        printf("- Standard Forward Pulse Laser\n");
                    } else {
                        printf("- Forward Pulse Laser\n");
                    }
                    forward_pulse_laser_found = true;
                }
            }
            // Rear-mounted Laser
            if (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON &&
                ((player_ship->equipment[i].typeSpecific.weaponType >= WEAPON_TYPE_PULSE_LASER &&
                  player_ship->equipment[i].typeSpecific.weaponType <= WEAPON_TYPE_MINING_LASER) ||
                 player_ship->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_REAR_LASER)) {
                if (!rear_laser_found) {
                    printf("- Rear-mounted Laser\n");
                    rear_laser_found = true;
                }
            }
            // Defensive Systems
            if (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
                player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
                if (player_ship->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM && !ecm_found) {
                    printf("- ECM Unit\n");
                    ecm_found = true;
                }
            }

            // Utility Systems
            if (player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
                player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4) {
                switch (player_ship->equipment[i].typeSpecific.utilityType) {
                case UTILITY_SYSTEM_TYPE_ESCAPE_POD:
                    if (!escape_pod_found) {
                        printf("- Escape Pod\n");
                        escape_pod_found = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_FUEL_SCOOPS:
                    if (!fuel_scoops_found) {
                        printf("- Fuel Scoops\n");
                        fuel_scoops_found = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER:
                    if (!docking_computer_found) {
                        printf("- Docking Computer\n");
                        docking_computer_found = true;
                    }
                    break;
                case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
                    if (!scanner_upgrade_found) {
                        printf("- Scanner Upgrade\n");
                        scanner_upgrade_found = true;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    printf("\n--- Cargo Hold (%dT used / %dT capacity) ---\n", player_ship->attributes.currentCargoTons,
           player_ship->attributes.cargoCapacityTons);
    bool has_cargo = false;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship->cargo[i].quantity > 0) {
            has_cargo = true;
            printf("- %s: %d units (Bought at: %dcr each)\n", player_ship->cargo[i].name,
                   player_ship->cargo[i].quantity, player_ship->cargo[i].purchasePrice);
        }
    }
    if (!has_cargo) {
        printf("Cargo hold is empty.\n");
    }
    printf("---------------------------\n");
    printf("\nEquipment inventory commands: 'inv', 'store <slot>', 'use <inv_idx> <slot>'\n");
}

/**
 * Refuels the player's ship.
 */
[[maybe_unused]] static inline double refuel_ship(player_ship_t *player_ship, double fuel_amount_ly,
                                                  bool use_fuel_scoops, bool external_sync) {
    if (player_ship == nullptr) {
        return 0.0;
    }

    const double MAX_FUEL_LY = player_ship->ship_type_t->maxFuelLY;
    double current_fuel_ly = player_ship->attributes.fuelLiters / 100.0;

    if (current_fuel_ly >= MAX_FUEL_LY) {
        printf("Fuel tanks already full (%.1f LY).\n", MAX_FUEL_LY);
        return 0.0;
    }

    double available_space = MAX_FUEL_LY - current_fuel_ly;
    double effective_request_ly = (fuel_amount_ly > available_space) ? available_space : fuel_amount_ly;

    if (use_fuel_scoops) {
        if (!has_fuel_scoops(player_ship)) {
            printf("Error: Your ship is not equipped with fuel scoops.\n");
            return 0.0;
        }

        player_ship->attributes.fuelLiters += (effective_request_ly * 100.0);
        printf("Successfully scooped %.1f LY of fuel from the star.\n", effective_request_ly);

        if (external_sync) {
            uint16_t current_max_fuel = (uint16_t)(MAX_FUEL_LY * 10.0);
            uint16_t fuel_to_add = (uint16_t)(effective_request_ly * 10.0);

            if (g_state.Fuel + fuel_to_add > current_max_fuel) {
                g_state.Fuel = current_max_fuel;
            } else {
                g_state.Fuel += fuel_to_add;
            }
        }

        return effective_request_ly;
    }

    int current_fuel_cost = get_fuel_cost();
    uint16_t fuel_units = (uint16_t)(effective_request_ly * 10.0);
    int total_cost = fuel_units * current_fuel_cost;

    if (external_sync && total_cost > g_state.Cash) {
        uint16_t affordable_units = (uint16_t)(g_state.Cash / current_fuel_cost);
        fuel_units = affordable_units;
        total_cost = fuel_units * current_fuel_cost;
        effective_request_ly = affordable_units / 10.0;

        if (fuel_units == 0) {
            printf("Insufficient credits to purchase fuel.\n");
            return 0.0;
        }
    }

    if (external_sync) {
        g_state.Cash -= total_cost;
    }

    player_ship->attributes.fuelLiters += (effective_request_ly * 100.0);

    if (external_sync) {
        uint16_t current_max_fuel = (uint16_t)(MAX_FUEL_LY * 10.0);
        if (g_state.Fuel + fuel_units > current_max_fuel) {
            g_state.Fuel = current_max_fuel;
        } else {
            g_state.Fuel += fuel_units;
        }
    }

    printf("Purchased %.1f LY of fuel for %d credits.\n", effective_request_ly, total_cost);
    return effective_request_ly;
}

/**
 * Checks if the ship has a specific type of equipment installed.
 */
[[maybe_unused]] static inline bool has_equipment(const player_ship_t *player_ship_t, equipment_slot_type_t slot_type,
                                                  equipment_type_specifics_t specific_type) {
    if (player_ship_t == nullptr) {
        return false;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (!player_ship_t->equipment[i].isActive) {
            continue;
        }

        if (player_ship_t->equipment[i].slotType == slot_type) {
            switch (slot_type) {
            case EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON:
            case EQUIPMENT_SLOT_TYPE_AFT_WEAPON:
                if (player_ship_t->equipment[i].typeSpecific.weaponType == specific_type.weaponType) {
                    return true;
                }
                break;

            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_1:
            case EQUIPMENT_SLOT_TYPE_DEFENSIVE_2:
                if (player_ship_t->equipment[i].typeSpecific.defensiveType == specific_type.defensiveType) {
                    return true;
                }
                break;

            case UTILITY_SYSTEM_1:
            case UTILITY_SYSTEM_2:
            case UTILITY_SYSTEM_3:
            case UTILITY_SYSTEM_4:
                if (player_ship_t->equipment[i].typeSpecific.utilityType == specific_type.utilityType) {
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
 */
[[maybe_unused]] static inline int repair_hull(player_ship_t *player_ship_t, int repair_amount, int cost_per_point,
                                               bool external_sync) {
    if (player_ship_t == nullptr) {
        return 0;
    }

    if (player_ship_t->attributes.hullStrength >= player_ship_t->ship_type_t->baseHullStrength) {
        printf("Hull already at maximum strength.\n");
        return 0;
    }

    int max_repair = player_ship_t->ship_type_t->baseHullStrength - player_ship_t->attributes.hullStrength;
    int effective_repair = (repair_amount > max_repair) ? max_repair : repair_amount;
    int total_cost = effective_repair * cost_per_point;

    if (external_sync) {
        if (total_cost > g_state.Cash) {
            effective_repair = (int)(g_state.Cash / cost_per_point);
            total_cost = effective_repair * cost_per_point;

            if (effective_repair == 0) {
                printf("Insufficient credits for hull repairs.\n");
                return 0;
            }
        }
        g_state.Cash -= total_cost;
    }

    player_ship_t->attributes.hullStrength += effective_repair;
    printf("Repaired %d hull strength points for %d credits.\n", effective_repair, total_cost);
    return effective_repair;
}

/**
 * Adds equipment to the player's ship.
 */
[[maybe_unused]] static inline bool add_equipment(player_ship_t *player_ship_t, equipment_slot_type_t slot_type,
                                                  const char *equipment_name, equipment_type_specifics_t specific_type,
                                                  double damage_output) {
    if (player_ship_t == nullptr || equipment_name == nullptr) {
        return false;
    }

    if (slot_type < 0 || slot_type >= MAX_EQUIPMENT_SLOTS) {
        printf("Error: Invalid equipment slot type %d.\n", slot_type);
        return false;
    }

    if (player_ship_t->equipment[slot_type].isActive) {
        char old_equip_name[MAX_SHIP_NAME_LENGTH];
        int old_equip_name_length =
            safe_snprintf(old_equip_name, MAX_SHIP_NAME_LENGTH, "%s", player_ship_t->equipment[slot_type].name);
        if (old_equip_name_length < 0) {
            old_equip_name[0] = '\0';
        }
        old_equip_name[MAX_SHIP_NAME_LENGTH - 1] = '\0';

        if (remove_equipment_to_inventory(player_ship_t, slot_type)) {
            // Successfully moved to inventory
        } else {
            printf(
                "Warning: Replacing existing equipment '%s' in slot %d without storing it (inventory may be full).\n",
                old_equip_name, slot_type);
            player_ship_t->equipment[slot_type].isActive = false;
            if (safe_snprintf(player_ship_t->equipment[slot_type].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
                player_ship_t->equipment[slot_type].name[0] = '\0';
            }
        }
    }

    player_ship_t->equipment[slot_type].isActive = true;
    int name_length = safe_snprintf(player_ship_t->equipment[slot_type].name, MAX_SHIP_NAME_LENGTH, "%s", equipment_name);
    if (name_length < 0) {
        player_ship_t->equipment[slot_type].isActive = false;
        player_ship_t->equipment[slot_type].name[0] = '\0';
        return false;
    }
    player_ship_t->equipment[slot_type].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    player_ship_t->equipment[slot_type].typeSpecific = specific_type;
    player_ship_t->equipment[slot_type].damageOutput = damage_output;

    printf("%s added to slot %d.\n", equipment_name, slot_type);
    return true;
}

/**
 * Removes equipment from the player's ship.
 */
[[maybe_unused]] static inline bool remove_equipment(player_ship_t *player_ship_t, equipment_slot_type_t slot_type) {
    if (player_ship_t == nullptr || slot_type >= MAX_EQUIPMENT_SLOTS) {
        return false;
    }

    if (!player_ship_t->equipment[slot_type].isActive) {
        printf("Error: No equipment installed in slot %d.\n", slot_type);
        return false;
    }

    char equipment_name[MAX_SHIP_NAME_LENGTH];
    int name_length = safe_snprintf(equipment_name, MAX_SHIP_NAME_LENGTH, "%s", player_ship_t->equipment[slot_type].name);
    if (name_length < 0) {
        return false;
    }
    equipment_name[MAX_SHIP_NAME_LENGTH - 1] = '\0';

    if (player_ship_t->equipment[slot_type].slotType >= UTILITY_SYSTEM_1 &&
        player_ship_t->equipment[slot_type].slotType <= UTILITY_SYSTEM_4) {
        if (player_ship_t->equipment[slot_type].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION) {
            if (player_ship_t->attributes.cargoCapacityTons - 5 < player_ship_t->attributes.currentCargoTons) {
                printf("Error: Can't remove cargo bay extension while cargo hold contains more than %d tons.\n",
                       player_ship_t->attributes.cargoCapacityTons - 5);
                return false;
            }
            player_ship_t->attributes.cargoCapacityTons -= 5;
        }
    }

    player_ship_t->equipment[slot_type].isActive = false;
    if (safe_snprintf(player_ship_t->equipment[slot_type].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
        return false;
    }

    printf("Successfully removed %s from slot %d.\n", equipment_name, slot_type);
    return true;
}

/**
 * Finds cargo by name.
 */
[[maybe_unused]] static inline int find_cargo_by_name(const player_ship_t *player_ship_t, const char *cargo_name) {
    if (player_ship_t == nullptr || cargo_name == nullptr) {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship_t->cargo[i].quantity > 0 && strcmp(player_ship_t->cargo[i].name, cargo_name) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the available cargo space.
 */
[[maybe_unused]] static inline int get_available_cargo_space(const player_ship_t *player_ship_t) {
    if (player_ship_t == nullptr) {
        return 0;
    }

    return player_ship_t->attributes.cargoCapacityTons - player_ship_t->attributes.currentCargoTons;
}

/**
 * Gets the total number of cargo items.
 */
[[maybe_unused]] static inline int get_cargo_item_count(const player_ship_t *player_ship_t) {
    if (player_ship_t == nullptr) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship_t->cargo[i].quantity > 0) {
            count++;
        }
    }

    return count;
}

/**
 * Gets the cargo item at a specific index.
 */
[[maybe_unused]] static inline bool get_cargo_item_at_index(const player_ship_t *player_ship_t, int index,
                                                            char *out_cargo_name, int *out_quantity,
                                                            int *out_purchase_price) {
    if (player_ship_t == nullptr || out_cargo_name == nullptr || out_quantity == nullptr ||
        out_purchase_price == nullptr) {
        return false;
    }

    if (index < 0 || index >= MAX_CARGO_SLOTS) {
        return false;
    }

    if (player_ship_t->cargo[index].quantity <= 0) {
        return false;
    }

    int name_length = safe_snprintf(out_cargo_name, MAX_SHIP_NAME_LENGTH, "%s", player_ship_t->cargo[index].name);
    if (name_length < 0 || name_length >= MAX_SHIP_NAME_LENGTH) {
        return false;
    }
    *out_quantity = player_ship_t->cargo[index].quantity;
    *out_purchase_price = player_ship_t->cargo[index].purchasePrice;

    return true;
}

/**
 * Displays detailed information about the cargo.
 */
[[maybe_unused]] static inline void display_cargo_details(const player_ship_t *player_ship_t) {
    if (player_ship_t == nullptr) {
        printf("Error: Ship data is nullptr.\n");
        return;
    }

    printf("\n=== Cargo Hold (%d/%d tons) ===\n", player_ship_t->attributes.currentCargoTons,
           player_ship_t->attributes.cargoCapacityTons);

    if (player_ship_t->attributes.currentCargoTons == 0) {
        printf("Cargo hold is empty.\n");
        return;
    }

    printf("%-20s %-10s %-15s %-15s\n", "Commodity", "Quantity", "Purchase Price", "Total Value");
    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");

    int total_items = 0;
    int total_value = 0;

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship_t->cargo[i].quantity > 0) {
            int item_total_value = player_ship_t->cargo[i].quantity * player_ship_t->cargo[i].purchasePrice;
            total_items += player_ship_t->cargo[i].quantity;
            total_value += item_total_value;

            printf("%-20s %-10d %-15d %-15d\n", player_ship_t->cargo[i].name, player_ship_t->cargo[i].quantity,
                   player_ship_t->cargo[i].purchasePrice, item_total_value);
        }
    }

    printf("%-20s %-10s %-15s %-15s\n", "----------", "--------", "--------------", "-----------");
    printf("%-20s %-10d %-15s %-15d\n", "TOTAL", total_items, "", total_value);
    printf("\nAvailable space: %d tons\n", get_available_cargo_space(player_ship_t));
}

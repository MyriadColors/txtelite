#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "elite_ship_components.h"
#include "elite_state.h"

/**
 * Check if the player's ship has fuel scoops installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if fuel scoops are installed, false otherwise
 */
[[maybe_unused]] static inline bool has_fuel_scoops(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive &&
            player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            player_ship->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_FUEL_SCOOPS) {
            return true;
        }
    }

    return false;
}

/**
 * Check if the player's ship has an ECM system installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if ECM is installed, false otherwise
 */
static inline bool has_ecm(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive &&
            (player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
             player_ship->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            player_ship->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM) {
            return true;
        }
    }

    return false;
}

/**
 * Check if the player's ship has a docking computer installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if docking computer is installed, false otherwise
 */
static inline bool has_docking_computer(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive &&
            player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            player_ship->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER) {
            return true;
        }
    }

    return false;
}

/**
 * Check if the player's ship has an upgraded scanner installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if upgraded scanner is installed, false otherwise
 */
static inline bool has_upgraded_scanner(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive &&
            player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            player_ship->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE) {
            return true;
        }
    }

    return false;
}

/**
 * Check if the player's ship has an escape pod installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if escape pod is installed, false otherwise
 */
static inline bool has_escape_pod(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (player_ship->equipment[i].isActive &&
            player_ship->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            player_ship->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            player_ship->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_ESCAPE_POD) {
            return true;
        }
    }

    return false;
}

/**
 * Activates ECM to destroy incoming enemy missiles.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if ECM was successfully activated, false otherwise
 */
[[maybe_unused]] static inline bool activate_ecm(player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    if (!has_ecm(player_ship)) {
        printf("Error: Your ship is not equipped with ECM System.\n");
        return false;
    }

    printf("ECM System activated! All incoming missiles have been destroyed.\n");
    return true;
}

/**
 * Activates the docking computer to automatically dock with a station.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @param distance The distance to the station (used to determine docking time)
 * @return true if docking computer was activated successfully, false otherwise
 */
[[maybe_unused]] static inline bool activate_docking_computer(player_ship_t *player_ship, double distance) {
    if (player_ship == nullptr) {
        return false;
    }

    if (!has_docking_computer(player_ship)) {
        printf("Error: Your ship is not equipped with a Docking Computer.\n");
        return false;
    }

    // Calculate docking time based on distance
    // This is a placeholder - actual docking procedure would be implemented elsewhere
    int docking_time_seconds = (int)(distance * 5.0); // 5 seconds per AU for example

    printf("Docking Computer activated. Auto-docking sequence initiated.\n");
    printf("Estimated time to complete docking: %d seconds.\n", docking_time_seconds);

    return true;
}

/**
 * Uses the ship's scanner to get enhanced information about nearby objects.
 * The quality and range of information depends on whether a scanner upgrade is installed.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @return true if scan was successful, false otherwise
 */
[[maybe_unused]] static inline bool use_scanner(player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    if (has_upgraded_scanner(player_ship)) {
        printf("Advanced scanner activated. Extended range and detailed scan initiated.\n");
    } else {
        printf("Basic scanner activated. Standard scan initiated.\n");
    }

    return true;
}

/**
 * Attempts to deploy the escape pod if the ship is critically damaged.
 * If successful, the player escapes but loses the ship and cargo.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @param critical_damage Whether the ship has taken critical damage
 * @return true if escape pod was successfully deployed, false otherwise
 */
[[maybe_unused]] static inline bool deploy_escape_pod(player_ship_t *player_ship, bool critical_damage) {
    if (player_ship == nullptr) {
        return false;
    }

    if (!has_escape_pod(player_ship)) {
        printf("Error: Your ship is not equipped with an Escape Pod.\n");
        return false;
    }

    // Only allow escape pod use if ship is critically damaged or override for testing
    if (!critical_damage) {
        printf("Escape pod can only be deployed in case of critical ship damage.\n");
        return false;
    }
    printf("EMERGENCY: Escape pod deployed! You have been safely ejected from your ship.\n");
    printf("Your ship and cargo have been lost, but you have survived.\n");

    return true;
}

/**
 * Gets the damage output of a specific weapon.
 * Used in combat calculations to determine damage dealt to targets.
 *
 * @param player_ship Pointer to the player_ship_t structure
 * @param slot_type The weapon slot to check (forward or aft)
 * @return The damage output value, or 0.0 if no weapon is installed
 */
[[maybe_unused]] static inline double get_weapon_damage(const player_ship_t *player_ship, equipment_slot_type_t slot_type) {
    if (player_ship == nullptr ||
        (slot_type != EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON && slot_type != EQUIPMENT_SLOT_TYPE_AFT_WEAPON)) {
        return 0.0;
    }

    // Check if the weapon slot has an active weapon
    if (player_ship->equipment[slot_type].isActive) {
        return player_ship->equipment[slot_type].damageOutput;
    }

    return 0.0;
}

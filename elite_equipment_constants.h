#pragma once

#include "elite_ship_components.h" // For player_ship_t and other types
#include "elite_state.h"
#include <string.h>

// Equipment slot indices for simplicity in status checks
// These are indices into the ship's equipment array for common equipment
#define EQUIP_ECM_SYSTEM 0
#define EQUIP_FUEL_SCOOP 1
#define EQUIP_DOCKING_COMPUTER 2
#define EQUIP_SCANNER_UPGRADE 3
#define EQUIP_ESCAPE_POD 4
#define EQUIP_MINING_LASER 5
#define EQUIP_BEAM_LASER 6
#define EQUIP_MILITARY_LASER 7

// Additional equipment status flags
#define EQUIP_STATUS_NOT_PRESENT 0
#define EQUIP_STATUS_PRESENT 1
#define EQUIP_STATUS_ACTIVE 2
#define EQUIP_STATUS_DAMAGED 3

/**
 * Checks if a specific equipment is present and active on the ship
 *
 * @param player_ship_t Pointer to the player's ship
 * @param equipIndex Equipment index to check
 * @return 1 if equipment is present and active, 0 otherwise
 */
[[maybe_unused]] static inline bool check_equipment_active(const struct player_ship_t *player_ship, int equip_index) {
    if (player_ship == nullptr || equip_index < 0 || equip_index >= MAX_EQUIPMENT_SLOTS) {
        return false;
    }

    return player_ship->equipment[equip_index].isActive;
}

/**
 * Maps equipment types to standard indices for easy access
 *
 * @param player_ship_t Pointer to the player's ship
 */
[[maybe_unused]] static inline void reset_equipment_indices(struct player_ship_t *player_ship) {
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        player_ship->equipment[i].isActive = false;
        if (i >= EQUIP_ECM_SYSTEM && i <= EQUIP_MILITARY_LASER) {
            memset(player_ship->equipment[i].name, 0, MAX_SHIP_NAME_LENGTH);
        }
    }
}

[[maybe_unused]] static inline void restore_active_equipment(player_ship_t *player_ship,
                                                             const ship_equipment_item_t *original_equipment) {
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (original_equipment[i].isActive) {
            player_ship->equipment[i] = original_equipment[i];
        }
    }
}

[[maybe_unused]] static inline void map_equipment_item(player_ship_t *player_ship, const ship_equipment_item_t *item) {
    if (!item->isActive) {
        return;
    }

    equipment_slot_type_t slot_type = item->slotType;
    if (slot_type == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 && item->typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM) {
        player_ship->equipment[EQUIP_ECM_SYSTEM].isActive = true;
        return;
    }

    if (slot_type >= UTILITY_SYSTEM_1 && slot_type <= UTILITY_SYSTEM_4) {
        int index = -1;
        switch (item->typeSpecific.utilityType) {
        case UTILITY_SYSTEM_TYPE_FUEL_SCOOPS:
            index = EQUIP_FUEL_SCOOP;
            break;
        case UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER:
            index = EQUIP_DOCKING_COMPUTER;
            break;
        case UTILITY_SYSTEM_TYPE_ESCAPE_POD:
            index = EQUIP_ESCAPE_POD;
            break;
        case UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE:
            index = EQUIP_SCANNER_UPGRADE;
            break;
        default:
            break;
        }
        if (index >= 0) {
            player_ship->equipment[index].isActive = true;
        }
        return;
    }

    if (slot_type == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON) {
        int index = -1;
        switch (item->typeSpecific.weaponType) {
        case WEAPON_TYPE_MINING_LASER:
            index = EQUIP_MINING_LASER;
            break;
        case WEAPON_TYPE_BEAM_LASER:
            index = EQUIP_BEAM_LASER;
            break;
        case WEAPON_TYPE_MILITARY_LASER:
            index = EQUIP_MILITARY_LASER;
            break;
        default:
            break;
        }
        if (index >= 0) {
            player_ship->equipment[index].isActive = true;
        }
    }
}

[[maybe_unused]] static inline void map_equipment_indices(player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return;
    }

    // Create a temporary backup of the equipment array
    ship_equipment_item_t original_equipment[MAX_EQUIPMENT_SLOTS];
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        original_equipment[i] = player_ship->equipment[i];
    }

    // Reset all equipment in our standard indices to make sure mapping is fresh
    reset_equipment_indices(player_ship);

    // Restore original equipment
    restore_active_equipment(player_ship, original_equipment);

    // Map equipment to our standard indices for easy status checks
    // These are logical mappings, not physical slot replacements
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        map_equipment_item(player_ship, &player_ship->equipment[i]);
    }
}

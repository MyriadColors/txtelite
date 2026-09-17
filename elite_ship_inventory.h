#pragma once

#include "elite_equipment_constants.h" // For map_equipment_indices
#include "elite_ship_components.h"     // Include the basic ship type definitions
#include "elite_state.h"
#include "platform_compat.h"
#include <stdio.h>
#include <string.h>

// Maximum equipment items the player can have in inventory (not equipped)
#define MAX_EQUIPMENT_INVENTORY 30

/**
 * Stores equipment in the ship's inventory when removed from a slot.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param equipment The equipment item to store
 * @return 1 if equipment was successfully stored, 0 if inventory is full
 */
static inline bool store_equipment_in_inventory(player_ship_t *player_ship, ship_equipment_item_t equipment) {
    if (player_ship == nullptr) {
        return false;
    }

    // Find first free inventory slot
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i) {
        if (!player_ship->equipmentInventory[i].isActive) {
            // Store equipment in inventory
            player_ship->equipmentInventory[i] = equipment;
            player_ship->equipmentInventory[i].isActive = true; // Mark as active in inventory
            printf("Stored %s in inventory slot %d.\n", equipment.name, i);
            return true;
        }
    }

    printf("Error: Equipment inventory is full. Cannot store %s.\n", equipment.name);
    return false;
}

/**
 * Remove equipment from a slot and store it in inventory.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param slotType The slot to remove equipment from
 * @return 1 if equipment was successfully removed and stored, 0 otherwise
 */
static inline bool remove_equipment_to_inventory(player_ship_t *player_ship, equipment_slot_type_t slot_type) {
    if (player_ship == nullptr || slot_type >= MAX_EQUIPMENT_SLOTS) {
        return false;
    }

    // Check if there's actually equipment installed
    if (!player_ship->equipment[slot_type].isActive) {
        printf("Error: No equipment installed in slot %d.\n", slot_type);
        return false;
    }

    // Save current equipment to add to inventory
    ship_equipment_item_t equip_to_store = player_ship->equipment[slot_type];
    char equipment_name[MAX_SHIP_NAME_LENGTH];
    int name_length = safe_snprintf(equipment_name, MAX_SHIP_NAME_LENGTH, "%s", equip_to_store.name);
    if (name_length < 0 || name_length >= MAX_SHIP_NAME_LENGTH) {
        printf("Error: Equipment name is invalid or too long.\n");
        return false;
    }

    // Special handling before removal
    if (slot_type >= UTILITY_SYSTEM_1 && slot_type <= UTILITY_SYSTEM_4) {
        // Reverse cargo bay extension effect
        if (player_ship->equipment[slot_type].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION) {
            // Check if removing cargo capacity would leave enough space for current cargo
            if (player_ship->attributes.cargoCapacityTons - 5 < player_ship->attributes.currentCargoTons) {
                printf("Error: Can't remove cargo bay extension while cargo hold contains more than %d tons.\n",
                       player_ship->attributes.cargoCapacityTons - 5);
                return false;
            }

            // Decrease cargo capacity
            player_ship->attributes.cargoCapacityTons -= 5;
        }
    }

    // Store equipment in inventory
    if (!store_equipment_in_inventory(player_ship, equip_to_store)) {
        return false; // Failed to store in inventory (inventory full)
    }

    // Reset the equipment slot
    player_ship->equipment[slot_type].isActive = false;
    memcpy(player_ship->equipment[slot_type].name, "Empty", sizeof("Empty"));
    // Leave other fields as they are - they'll be overwritten on next install

    printf("Successfully removed %s from slot %d and stored in inventory.\n", equipment_name, slot_type);

    // Update equipment mapping for quick access
    map_equipment_indices(player_ship);

    return true;
}

/**
 * Equips an item from the inventory into an equipment slot.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param inventoryIndex Index of the equipment in inventory
 * @param slotType The slot to equip the item to
 * @return 1 if equipment was successfully equipped, 0 otherwise
 */
[[maybe_unused]] static inline bool equip_from_inventory(player_ship_t *player_ship, int inventory_index,
                                                         equipment_slot_type_t slot_type) {
    if (player_ship == nullptr || inventory_index < 0 || inventory_index >= MAX_EQUIPMENT_INVENTORY || slot_type < 0 ||
        slot_type >= MAX_EQUIPMENT_SLOTS) {
        return false;
    }

    // Check if the inventory slot has equipment
    if (!player_ship->equipmentInventory[inventory_index].isActive) {
        printf("Error: No equipment in inventory slot %d.\n", inventory_index);
        return false;
    }

    // Get the inventory equipment
    ship_equipment_item_t inventory_equipment = player_ship->equipmentInventory[inventory_index];

    // Verify equipment compatibility with the target slot
    bool is_compatible = false;

    // Check if the target slot and the equipment slot type are compatible
    // We organize the slot types by category and verify they are in the same category
    if (slot_type == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON || slot_type == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
        // For weapons, check if the original slot was any weapon slot
        if (inventory_equipment.slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
            inventory_equipment.slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
            is_compatible = true;
        }
    } else if (slot_type == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 || slot_type == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
        // For defensive equipment, check if the original slot was any defensive slot
        if (inventory_equipment.slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
            inventory_equipment.slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
            is_compatible = true;
        }
    } else if (slot_type >= UTILITY_SYSTEM_1 && slot_type <= UTILITY_SYSTEM_4) {
        // For utility systems, check if the original slot was any utility slot
        if (inventory_equipment.slotType >= UTILITY_SYSTEM_1 && inventory_equipment.slotType <= UTILITY_SYSTEM_4) {
            is_compatible = true;
        }
    }

    if (!is_compatible) {
        printf("Error: %s cannot be installed in slot %d. Incorrect slot type.\n", inventory_equipment.name, slot_type);
        return false;
    }

    // Check if the target slot is already occupied
    if (player_ship->equipment[slot_type].isActive) {
        // Move the current equipment to inventory before replacing
        if (!remove_equipment_to_inventory(player_ship, slot_type)) {
            // If we couldn't store the current equipment (inventory full), abort
            return false;
        }
    }

    // Install the equipment from inventory
    player_ship->equipment[slot_type] = inventory_equipment;

    // If this is a cargo bay extension, add the extra cargo capacity
    if (slot_type >= UTILITY_SYSTEM_1 && slot_type <= UTILITY_SYSTEM_4 &&
        inventory_equipment.typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION) {
        player_ship->attributes.cargoCapacityTons += 5;
    }

    // Clear the inventory slot
    player_ship->equipmentInventory[inventory_index].isActive = false;
    if (safe_snprintf(player_ship->equipmentInventory[inventory_index].name, MAX_SHIP_NAME_LENGTH, "Empty") < 0) {
        return false;
    }

    printf("Equipped %s from inventory to slot %d.\n", inventory_equipment.name, slot_type);

    // Update equipment mapping for quick access
    map_equipment_indices(player_ship);

    return true;
}

/**
 * Lists all equipment stored in the inventory.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 */
[[maybe_unused]] static inline void list_equipment_inventory(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return;
    }

    printf("\n--- Equipment Inventory ---\n");

    bool has_inventory = false;
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i) {
        if (player_ship->equipmentInventory[i].isActive) {
            has_inventory = true;

            // Determine item type for better display
            const char *item_type = "Unknown";
            equipment_slot_type_t slot_type = player_ship->equipmentInventory[i].slotType;

            if (slot_type == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON || slot_type == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
                item_type = "Weapon";
            } else if (slot_type == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 || slot_type == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
                item_type = "Defensive";
            } else if (slot_type >= UTILITY_SYSTEM_1 && slot_type <= UTILITY_SYSTEM_4) {
                item_type = "Utility";
            }

            printf("[%2d] %s (Type: %s)\n", i, player_ship->equipmentInventory[i].name, item_type);
        }
    }

    if (!has_inventory) {
        printf("No equipment in inventory.\n");
    }
    printf("---------------------------\n");
    printf("Use 'use <inventory_index> <equipment_slot>' to install equipment from inventory.\n");
    printf("Example: use 0 1  (equips item from inventory slot 0 to equipment slot 1)\n");
    printf("Available equipment slots: Forward Weapon (1), Aft Weapon (2), Defensive (3-4), Utility (5-8)\n");
}

/**
 * Prints the equipment slots of the player's ship.
 *
 * @param player_ship Pointer to the player_ship_tstructure
 */
[[maybe_unused]] static inline void print_equipment_slots(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return;
    }

    printf("\n--- Equipment Slots ---\n");

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        char slot_type_name[30] = "Unknown";
        int written = 0;

        // Determine slot type name
        if (i == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON) {
            written = safe_snprintf(slot_type_name, sizeof(slot_type_name), "Forward Weapon");
        } else if (i == EQUIPMENT_SLOT_TYPE_AFT_WEAPON) {
            written = safe_snprintf(slot_type_name, sizeof(slot_type_name), "Aft Weapon");
        } else if (i == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1) {
            written = safe_snprintf(slot_type_name, sizeof(slot_type_name), "Defensive System 1");
        } else if (i == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) {
            written = safe_snprintf(slot_type_name, sizeof(slot_type_name), "Defensive System 2");
        } else if (i >= UTILITY_SYSTEM_1 && i <= UTILITY_SYSTEM_4) {
            written = safe_snprintf(slot_type_name, sizeof(slot_type_name), "Utility System %d", (i - UTILITY_SYSTEM_1) + 1);
        }

        if (written < 0 || (size_t)written >= sizeof(slot_type_name)) {
            slot_type_name[0] = '\0';
        }

        // Print slot info
        printf("\nSlot %d (%s): %s", i, slot_type_name,
               (int)player_ship->equipment[i].isActive ? player_ship->equipment[i].name : "Empty");
    }

    printf("\n");
}

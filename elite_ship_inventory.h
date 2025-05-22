#pragma once

#include "elite_ship_types.h" // Include the basic ship type definitions

// Maximum equipment items the player can have in inventory (not equipped)
#define MAX_EQUIPMENT_INVENTORY 30

/**
 * Stores equipment in the ship's inventory when removed from a slot.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param equipment The equipment item to store
 * @return true if equipment was successfully stored, false if inventory is full
 */
inline bool StoreEquipmentInInventory(PlayerShip *playerShip, ShipEquipmentItem equipment)
{
    if (playerShip == NULL)
    {
        return false;
    }

    // Find first free inventory slot
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i)
    {
        if (!playerShip->equipmentInventory[i].isActive)
        {
            // Store equipment in inventory
            playerShip->equipmentInventory[i] = equipment;
            playerShip->equipmentInventory[i].isActive = 1; // Mark as active in inventory
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
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The slot to remove equipment from
 * @return true if equipment was successfully removed and stored, false otherwise
 */
inline bool RemoveEquipmentToInventory(PlayerShip *playerShip, EquipmentSlotType slotType)
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

    // Save current equipment to add to inventory
    ShipEquipmentItem equipToStore = playerShip->equipment[slotType];
    char equipmentName[MAX_SHIP_NAME_LENGTH];
    strncpy(equipmentName, equipToStore.name, MAX_SHIP_NAME_LENGTH - 1);
    equipmentName[MAX_SHIP_NAME_LENGTH - 1] = '\0';

    // Special handling before removal
    if (slotType >= UTILITY_SYSTEM_1 && slotType <= UTILITY_SYSTEM_4)
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

    // Store equipment in inventory
    if (!StoreEquipmentInInventory(playerShip, equipToStore))
    {
        return false; // Failed to store in inventory (inventory full)
    }

    // Reset the equipment slot
    playerShip->equipment[slotType].isActive = 0;
    strcpy(playerShip->equipment[slotType].name, "Empty");
    // Leave other fields as they are - they'll be overwritten on next install

    printf("Successfully removed %s from slot %d and stored in inventory.\n", equipmentName, slotType);

    // Update equipment mapping for quick access
    MapEquipmentIndices(playerShip);

    return true;
}

/**
 * Equips an item from the inventory into an equipment slot.
 *
 * @param playerShip Pointer to the PlayerShip structure
 * @param inventoryIndex Index of the equipment in inventory
 * @param slotType The slot to equip the item to
 * @return true if equipment was successfully equipped, false otherwise
 */
inline bool EquipFromInventory(PlayerShip *playerShip, int inventoryIndex, EquipmentSlotType slotType)
{
    if (playerShip == NULL || inventoryIndex < 0 || inventoryIndex >= MAX_EQUIPMENT_INVENTORY || slotType < 0 || slotType >= MAX_EQUIPMENT_SLOTS)
    {
        return false;
    }

    // Check if the inventory slot has equipment
    if (!playerShip->equipmentInventory[inventoryIndex].isActive)
    {
        printf("Error: No equipment in inventory slot %d.\n", inventoryIndex);
        return false;
    }

    // Get the inventory equipment
    ShipEquipmentItem inventoryEquipment = playerShip->equipmentInventory[inventoryIndex];

    // Verify equipment compatibility with the target slot
    bool isCompatible = false;

    // Check if the target slot and the equipment slot type are compatible
    // We organize the slot types by category and verify they are in the same category
    if (slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON || slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
    {
        // For weapons, check if the original slot was any weapon slot
        if (inventoryEquipment.slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON ||
            inventoryEquipment.slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
        {
            isCompatible = true;
        }
    }
    else if (slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 || slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
    {
        // For defensive equipment, check if the original slot was any defensive slot
        if (inventoryEquipment.slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
            inventoryEquipment.slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
        {
            isCompatible = true;
        }
    }
    else if (slotType >= UTILITY_SYSTEM_1 && slotType <= UTILITY_SYSTEM_4)
    {
        // For utility systems, check if the original slot was any utility slot
        if (inventoryEquipment.slotType >= UTILITY_SYSTEM_1 &&
            inventoryEquipment.slotType <= UTILITY_SYSTEM_4)
        {
            isCompatible = true;
        }
    }

    if (!isCompatible)
    {
        printf("Error: %s cannot be installed in slot %d. Incorrect slot type.\n",
               inventoryEquipment.name, slotType);
        return false;
    }

    // Check if the target slot is already occupied
    if (playerShip->equipment[slotType].isActive)
    {
        // Move the current equipment to inventory before replacing
        if (!RemoveEquipmentToInventory(playerShip, slotType))
        {
            // If we couldn't store the current equipment (inventory full), abort
            return false;
        }
    }

    // Install the equipment from inventory
    playerShip->equipment[slotType] = inventoryEquipment;

    // If this is a cargo bay extension, add the extra cargo capacity
    if (slotType >= UTILITY_SYSTEM_1 && slotType <= UTILITY_SYSTEM_4 &&
        inventoryEquipment.typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION)
    {
        playerShip->attributes.cargoCapacityTons += 5;
    }

    // Clear the inventory slot
    playerShip->equipmentInventory[inventoryIndex].isActive = 0;
    strcpy(playerShip->equipmentInventory[inventoryIndex].name, "Empty");

    printf("Equipped %s from inventory to slot %d.\n", inventoryEquipment.name, slotType);

    // Update equipment mapping for quick access
    MapEquipmentIndices(playerShip);

    return true;
}

/**
 * Lists all equipment stored in the inventory.
 *
 * @param playerShip Pointer to the PlayerShip structure
 */
inline void ListEquipmentInventory(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    printf("\n--- Equipment Inventory ---\n");

    bool hasInventory = false;
    for (int i = 0; i < MAX_EQUIPMENT_INVENTORY; ++i)
    {
        if (playerShip->equipmentInventory[i].isActive)
        {
            hasInventory = true;

            // Determine item type for better display
            char itemType[20] = "Unknown";
            EquipmentSlotType slotType = playerShip->equipmentInventory[i].slotType;

            if (slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON || slotType == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
            {
                strcpy(itemType, "Weapon");
            }
            else if (slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 || slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
            {
                strcpy(itemType, "Defensive");
            }
            else if (slotType >= UTILITY_SYSTEM_1 && slotType <= UTILITY_SYSTEM_4)
            {
                strcpy(itemType, "Utility");
            }

            printf("[%2d] %s (Type: %s)\n", i, playerShip->equipmentInventory[i].name, itemType);
        }
    }

    if (!hasInventory)
    {
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
 * @param playerShip Pointer to the PlayerShip structure
 */
inline void PrintEquipmentSlots(const PlayerShip *playerShip)
{
    if (playerShip == NULL)
    {
        return;
    }

    printf("\n--- Equipment Slots ---\n");

    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i)
    {
        char slotTypeName[30] = "Unknown";

        // Determine slot type name
        if (i == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON)
        {
            strcpy(slotTypeName, "Forward Weapon");
        }
        else if (i == EQUIPMENT_SLOT_TYPE_AFT_WEAPON)
        {
            strcpy(slotTypeName, "Aft Weapon");
        }
        else if (i == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1)
        {
            strcpy(slotTypeName, "Defensive System 1");
        }
        else if (i == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2)
        {
            strcpy(slotTypeName, "Defensive System 2");
        }
        else if (i >= UTILITY_SYSTEM_1 && i <= UTILITY_SYSTEM_4)
        {
            sprintf(slotTypeName, "Utility System %d", (i - UTILITY_SYSTEM_1) + 1);
        }

        // Print slot info
        printf("\nSlot %d (%s): %s",
               i,
               slotTypeName,
               playerShip->equipment[i].isActive ? playerShip->equipment[i].name : "Empty");
    }

    printf("\n");
}

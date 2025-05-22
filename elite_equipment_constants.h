#pragma once

#include "elite_ship_types.h" // For PlayerShip and other types

// Equipment slot indices for simplicity in status checks
// These are indices into the ship's equipment array for common equipment
#define EQUIP_ECM_SYSTEM          0
#define EQUIP_FUEL_SCOOP          1
#define EQUIP_ENERGY_BOMB         2 
#define EQUIP_DOCKING_COMPUTER    3
#define EQUIP_SCANNER_UPGRADE     4
#define EQUIP_ESCAPE_POD          5
#define EQUIP_MINING_LASER        6
#define EQUIP_BEAM_LASER          7
#define EQUIP_MILITARY_LASER      8

// Additional equipment status flags
#define EQUIP_STATUS_NOT_PRESENT  0
#define EQUIP_STATUS_PRESENT      1
#define EQUIP_STATUS_ACTIVE       2
#define EQUIP_STATUS_DAMAGED      3

/**
 * Checks if a specific equipment is present and active on the ship
 * 
 * @param playerShip Pointer to the player's ship
 * @param equipIndex Equipment index to check
 * @return true if equipment is present and active, false otherwise
 */
static inline bool CheckEquipmentActive(const PlayerShip* playerShip, int equipIndex) {
    if (playerShip == NULL || equipIndex < 0 || equipIndex >= MAX_EQUIPMENT_SLOTS) {
        return false;
    }
    
    return playerShip->equipment[equipIndex].isActive;
}

/**
 * Maps equipment types to standard indices for easy access
 * 
 * @param playerShip Pointer to the player's ship
 */
static inline void MapEquipmentIndices(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return;
    }
    
    // Create a temporary backup of the equipment array
    ShipEquipmentItem originalEquipment[MAX_EQUIPMENT_SLOTS];
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        originalEquipment[i] = playerShip->equipment[i];
    }
    
    // Reset all equipment in our standard indices to make sure mapping is fresh
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        playerShip->equipment[i].isActive = 0;
        if (i >= EQUIP_ECM_SYSTEM && i <= EQUIP_MILITARY_LASER) {
            // Only reset specialized equipment slots, not the original equipment slots
            memset(playerShip->equipment[i].name, 0, MAX_SHIP_NAME_LENGTH);
        }
    }
    
    // Restore original equipment
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (originalEquipment[i].isActive) {
            playerShip->equipment[i] = originalEquipment[i];
        }
    }
    
    // Map equipment to our standard indices for easy status checks
    // These are logical mappings, not physical slot replacements
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        if (playerShip->equipment[i].isActive) {
            EquipmentSlotType slotType = playerShip->equipment[i].slotType;
            
            if (slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 && 
                playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM) {
                // Map to ECM standard index
                playerShip->equipment[EQUIP_ECM_SYSTEM].isActive = 1;
            }
            else if (slotType >= UTILITY_SYSTEM_1 && slotType <= UTILITY_SYSTEM_4) {
                if (playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_FUEL_SCOOPS) {
                    playerShip->equipment[EQUIP_FUEL_SCOOP].isActive = 1;
                }
                else if (playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER) {
                    playerShip->equipment[EQUIP_DOCKING_COMPUTER].isActive = 1;
                }                else if (playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_ESCAPE_POD) {
                    playerShip->equipment[EQUIP_ESCAPE_POD].isActive = 1;
                }
                else if (playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE) {
                    playerShip->equipment[EQUIP_SCANNER_UPGRADE].isActive = 1;
                }
            }
            else if (slotType == EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON) {
                if (playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_MINING_LASER) {
                    playerShip->equipment[EQUIP_MINING_LASER].isActive = 1;
                }
                else if (playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_BEAM_LASER) {
                    playerShip->equipment[EQUIP_BEAM_LASER].isActive = 1;
                }
                else if (playerShip->equipment[i].typeSpecific.weaponType == WEAPON_TYPE_MILITARY_LASER) {
                    playerShip->equipment[EQUIP_MILITARY_LASER].isActive = 1;
                }
            }
        }
    }
}

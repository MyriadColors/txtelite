#pragma once

#include "elite_ship_types.h" // Include the basic ship type definitions

/**
 * Check if the player's ship has fuel scoops installed.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if fuel scoops are installed, false otherwise
 */
inline bool HasFuelScoops(const PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 && 
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_FUEL_SCOOPS) {
            return true;
        }
    }
    
    return false;
}

/**
 * Check if the player's ship has an ECM system installed.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if ECM is installed, false otherwise
 */
inline bool HasECM(const PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 || 
             playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM) {
            return true;
        }
    }
    
    return false;
}

/**
 * Check if the player's ship has a docking computer installed.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if docking computer is installed, false otherwise
 */
inline bool HasDockingComputer(const PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Iterate through equipment slots
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 && 
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER) {
            return true;
        }
    }
    
    return false;
}

/**
 * Activates ECM to destroy incoming enemy missiles.
 * Energy is required to operate the ECM.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if ECM was successfully activated, false otherwise
 */
inline bool ActivateECM(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Check if ship has ECM
    bool hasECM = false;
    double ecmEnergyCost = 0.0;
    
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            (playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_1 ||
             playerShip->equipment[i].slotType == EQUIPMENT_SLOT_TYPE_DEFENSIVE_2) &&
            playerShip->equipment[i].typeSpecific.defensiveType == DEFENSIVE_SYSTEM_TYPE_ECM) {
            hasECM = true;
            ecmEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }
    
    if (!hasECM) {
        printf("Error: Your ship is not equipped with ECM System.\n");
        return false;
    }
    
    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < ecmEnergyCost) {
        printf("Error: Insufficient energy to activate ECM System.\n");
        printf("Required: %.1f, Available: %.1f\n", ecmEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }
    
    // Consume energy
    playerShip->attributes.energyBanks -= ecmEnergyCost;
    
    printf("ECM System activated! All incoming missiles have been destroyed.\n");
    return true;
}

/**
 * Activates the docking computer to automatically dock with a station.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param distance The distance to the station (used to determine docking time)
 * @return true if docking computer was activated successfully, false otherwise
 */
inline bool ActivateDockingComputer(PlayerShip* playerShip, double distance) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Check if ship has docking computer
    bool hasDockingComputer = false;
    double dockingComputerEnergyCost = 0.0;
    
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER) {
            hasDockingComputer = true;
            dockingComputerEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }
    
    if (!hasDockingComputer) {
        printf("Error: Your ship is not equipped with a Docking Computer.\n");
        return false;
    }
    
    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < dockingComputerEnergyCost) {
        printf("Error: Insufficient energy to activate Docking Computer.\n");
        printf("Required: %.1f, Available: %.1f\n", dockingComputerEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }
    
    // Consume energy
    playerShip->attributes.energyBanks -= dockingComputerEnergyCost;
    
    // Calculate docking time based on distance
    // This is a placeholder - actual docking procedure would be implemented elsewhere
    int dockingTimeSeconds = (int)(distance * 5.0); // 5 seconds per AU for example
    
    printf("Docking Computer activated. Auto-docking sequence initiated.\n");
    printf("Estimated time to complete docking: %d seconds.\n", dockingTimeSeconds);
    
    // Here we'd normally advance the game time by dockingTimeSeconds
    // and trigger the actual docking process
    
    return true;
}

/**
 * Uses the ship's scanner to get enhanced information about nearby objects.
 * The quality and range of information depends on whether a scanner upgrade is installed.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return true if scan was successful, false otherwise
 */
inline bool UseScanner(PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Check if ship has advanced scanner
    bool hasUpgradedScanner = false;
    double scannerEnergyCost = 2.0; // Basic scanner energy cost
    
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE) {
            hasUpgradedScanner = true;
            scannerEnergyCost = playerShip->equipment[i].energyDraw;
            break;
        }
    }
    
    // Check if there's enough energy
    if (playerShip->attributes.energyBanks < scannerEnergyCost) {
        printf("Error: Insufficient energy to power scanner.\n");
        printf("Required: %.1f, Available: %.1f\n", scannerEnergyCost, playerShip->attributes.energyBanks);
        return false;
    }
    
    // Consume energy
    playerShip->attributes.energyBanks -= scannerEnergyCost;
    
    // Perform scan
    if (hasUpgradedScanner) {
        printf("Advanced scanner activated. Extended range and detailed scan initiated.\n");
        // Advanced scanner would provide more detailed information
        // This would normally integrate with the navigation system
    } else {
        printf("Basic scanner activated. Standard scan initiated.\n");
        // Basic scanner would provide standard information
    }
    
    return true;
}

/**
 * Attempts to deploy the escape pod if the ship is critically damaged.
 * If successful, the player escapes but loses the ship and cargo.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param criticalDamage Whether the ship has taken critical damage
 * @return true if escape pod was successfully deployed, false otherwise
 */
inline bool DeployEscapePod(PlayerShip* playerShip, bool criticalDamage) {
    if (playerShip == NULL) {
        return false;
    }
    
    // Check if ship has escape pod
    bool hasEscapePod = false;
    
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; ++i) {
        if (playerShip->equipment[i].isActive && 
            playerShip->equipment[i].slotType >= UTILITY_SYSTEM_1 &&
            playerShip->equipment[i].slotType <= UTILITY_SYSTEM_4 &&
            playerShip->equipment[i].typeSpecific.utilityType == UTILITY_SYSTEM_TYPE_ESCAPE_POD) {
            hasEscapePod = true;
            break;
        }
    }
    
    if (!hasEscapePod) {
        printf("Error: Your ship is not equipped with an Escape Pod.\n");
        return false;
    }
    
    // Only allow escape pod use if ship is critically damaged or override for testing
    if (!criticalDamage) {
        printf("Escape pod can only be deployed in case of critical ship damage.\n");
        return false;
    }
      printf("EMERGENCY: Escape pod deployed! You have been safely ejected from your ship.\n");
    printf("Your ship and cargo have been lost, but you have survived.\n");
    
    // This would normally trigger game logic to handle the aftermath
    // such as losing the ship and cargo, but preserving the player's life and credits
    
    return true;
}

/**
 * Gets the damage output of a specific weapon.
 * Used in combat calculations to determine damage dealt to targets.
 * 
 * @param playerShip Pointer to the PlayerShip structure
    
    // This would normally trigger the actual galaxy jump logic
    printf("Galactic Hyperspace Drive activated!\n");
    printf("Jumping to %s system...\n", targetSystemName);
    
    return true;
}

/**
 * Gets the damage output of a specific weapon.
 * Used in combat calculations to determine damage dealt to targets.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The weapon slot to check (forward or aft)
 * @return The damage output value, or 0.0 if no weapon is installed
 */
inline double GetWeaponDamage(const PlayerShip* playerShip, EquipmentSlotType slotType) {
    if (playerShip == NULL || 
        (slotType != EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON && slotType != EQUIPMENT_SLOT_TYPE_AFT_WEAPON)) {
        return 0.0;
    }
    
    // Check if the weapon slot has an active weapon
    if (playerShip->equipment[slotType].isActive) {
        return playerShip->equipment[slotType].damageOutput;
    }
    
    return 0.0;
}

/**
 * Gets the energy draw of a specific equipment item.
 * Used to calculate energy consumption during various operations.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param slotType The equipment slot to check
 * @return The energy draw value, or 0.0 if no equipment is installed
 */
inline double GetEquipmentEnergyDraw(const PlayerShip* playerShip, EquipmentSlotType slotType) {
    if (playerShip == NULL || slotType >= MAX_EQUIPMENT_SLOTS) {
        return 0.0;
    }
    
    // Check if the slot has active equipment
    if (playerShip->equipment[slotType].isActive) {
        return playerShip->equipment[slotType].energyDraw;
    }
    
    return 0.0;
}

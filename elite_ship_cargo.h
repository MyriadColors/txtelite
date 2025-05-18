#pragma once

#include "elite_ship_types.h"
#include <string.h> // For string functions
#include <stdio.h>  // For printf

/**
 * Find a cargo slot in the ship for a specific commodity
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to look for
 * @return Index of the cargo slot if found, -1 if not found
 */
inline int FindCargoSlot(const PlayerShip* playerShip, const char* cargoName) {
    if (playerShip == NULL || cargoName == NULL) {
        return -1;
    }
    
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (playerShip->cargo[i].quantity > 0 && strcmp(playerShip->cargo[i].name, cargoName) == 0) {
            return i;
        }
    }
    
    return -1; // Not found
}

/**
 * Find the first empty cargo slot in the ship
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @return Index of the first empty cargo slot if available, -1 if all slots are filled
 */
inline int FindEmptyCargoSlot(const PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return -1;
    }
    
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (playerShip->cargo[i].quantity == 0) {
            return i;
        }
    }
    
    return -1; // No empty slots
}

/**
 * Add cargo to the player's ship. If the cargo already exists, it will increase the quantity.
 * If the cargo doesn't exist, it will be added to the first empty slot.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to add
 * @param quantity Amount of cargo to add (in tonnes)
 * @param purchasePrice Price per tonne (for player's reference)
 * @return true if cargo was successfully added, false if there was no space
 */
inline bool AddCargo(PlayerShip* playerShip, const char* cargoName, int quantity, int purchasePrice) {
    if (playerShip == NULL || cargoName == NULL || quantity <= 0) {
        return false;
    }
    
    // Check if there's enough cargo capacity
    if (playerShip->attributes.currentCargoTons + quantity > playerShip->attributes.cargoCapacityTons) {
        printf("Error: Not enough cargo space. Available: %d tonnes, Needed: %d tonnes\n", 
               playerShip->attributes.cargoCapacityTons - playerShip->attributes.currentCargoTons, 
               quantity);
        return false;
    }
    
    // Check if we already have this cargo type
    int cargoSlot = FindCargoSlot(playerShip, cargoName);
    
    if (cargoSlot >= 0) {
        // Cargo already exists, increase quantity
        playerShip->cargo[cargoSlot].quantity += quantity;
        
        // Update purchase price as the average of the previous and new price
        // This gives a weighted average of purchase prices
        playerShip->cargo[cargoSlot].purchasePrice = 
            (playerShip->cargo[cargoSlot].purchasePrice * (playerShip->cargo[cargoSlot].quantity - quantity) + 
             purchasePrice * quantity) / playerShip->cargo[cargoSlot].quantity;
    } else {
        // Need to find an empty slot for the new cargo type
        cargoSlot = FindEmptyCargoSlot(playerShip);
        
        if (cargoSlot < 0) {
            printf("Error: No available cargo slots. Maximum different cargo types reached.\n");
            return false;
        }
        
        // Add new cargo type
        strncpy(playerShip->cargo[cargoSlot].name, cargoName, MAX_SHIP_NAME_LENGTH - 1);
        playerShip->cargo[cargoSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
        playerShip->cargo[cargoSlot].quantity = quantity;
        playerShip->cargo[cargoSlot].purchasePrice = purchasePrice;
    }
    
    // Update current cargo weight
    playerShip->attributes.currentCargoTons += quantity;
    
    printf("Added %d tonnes of %s to cargo hold.\n", quantity, cargoName);
    return true;
}

/**
 * Remove cargo from the player's ship.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to remove
 * @param quantity Amount of cargo to remove (in tonnes)
 * @return true if cargo was successfully removed, false if the ship doesn't have that cargo
 */
inline bool RemoveCargo(PlayerShip* playerShip, const char* cargoName, int quantity) {
    if (playerShip == NULL || cargoName == NULL || quantity <= 0) {
        return false;
    }
    
    // Find the cargo slot
    int cargoSlot = FindCargoSlot(playerShip, cargoName);
    
    if (cargoSlot < 0) {
        printf("Error: %s not found in cargo hold.\n", cargoName);
        return false;
    }
    
    // Check if we have enough of this cargo
    if (playerShip->cargo[cargoSlot].quantity < quantity) {
        printf("Error: Not enough %s in cargo hold. Available: %d tonnes, Requested: %d tonnes\n", 
               cargoName, playerShip->cargo[cargoSlot].quantity, quantity);
        return false;
    }
    
    // Remove the cargo
    playerShip->cargo[cargoSlot].quantity -= quantity;
    
    // Update current cargo weight
    playerShip->attributes.currentCargoTons -= quantity;
    
    // If quantity is now 0, clear the slot
    if (playerShip->cargo[cargoSlot].quantity == 0) {
        memset(&playerShip->cargo[cargoSlot], 0, sizeof(CargoItem));
        strncpy(playerShip->cargo[cargoSlot].name, "Empty", MAX_SHIP_NAME_LENGTH - 1);
        playerShip->cargo[cargoSlot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
    }
    
    printf("Removed %d tonnes of %s from cargo hold.\n", quantity, cargoName);
    return true;
}

/**
 * Sell cargo from the player's ship at a trading port and earn credits.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to sell
 * @param quantity Amount of cargo to sell (in tonnes)
 * @param salePrice Price per tonne that will be paid
 * @param externalSync If true, synchronize with the global state (Cash)
 * @return true if cargo was successfully sold, false if there was an error
 */
inline bool SellCargo(PlayerShip* playerShip, const char* cargoName, int quantity, int salePrice, bool externalSync) {
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || salePrice < 0) {
        return false;
    }
    
    // Try to remove the cargo
    if (!RemoveCargo(playerShip, cargoName, quantity)) {
        return false;
    }
    
    // Calculate the total sale amount
    int totalSale = quantity * salePrice;
    
    // If we need to sync with the game's global state
    if (externalSync) {
        extern int32_t Cash;
        Cash += totalSale;
    }
    
    printf("Sold %d tonnes of %s for %d credits.\n", quantity, cargoName, totalSale);
    return true;
}

/**
 * Buy cargo for the player's ship from a trading port.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to buy
 * @param quantity Amount of cargo to buy (in tonnes)
 * @param purchasePrice Price per tonne that must be paid
 * @param externalSync If true, synchronize with the global state (Cash)
 * @return true if cargo was successfully purchased, false if there was an error
 */
inline bool BuyCargo(PlayerShip* playerShip, const char* cargoName, int quantity, int purchasePrice, bool externalSync) {
    if (playerShip == NULL || cargoName == NULL || quantity <= 0 || purchasePrice < 0) {
        return false;
    }
    
    // Calculate the total cost
    int totalCost = quantity * purchasePrice;
    
    // If we need to sync with the game's global state, check if we have enough cash
    if (externalSync) {
        extern int32_t Cash;
        
        if (Cash < totalCost) {
            printf("Error: Not enough credits. Available: %d, Required: %d\n", Cash, totalCost);
            return false;
        }
    }
    
    // Try to add the cargo
    if (!AddCargo(playerShip, cargoName, quantity, purchasePrice)) {
        return false;
    }
    
    // If everything successful and we need to sync, deduct the cash
    if (externalSync) {
        extern int32_t Cash;
        Cash -= totalCost;
    }
    
    printf("Purchased %d tonnes of %s for %d credits.\n", quantity, cargoName, totalCost);
    return true;
}

/**
 * List all cargo items in the player's cargo hold.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 */
inline void ListCargo(const PlayerShip* playerShip) {
    if (playerShip == NULL) {
        return;
    }
    
    printf("\n--- Cargo Hold (%d/%d tonnes) ---\n", 
           playerShip->attributes.currentCargoTons, 
           playerShip->attributes.cargoCapacityTons);
    
    bool hasCargo = false;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (playerShip->cargo[i].quantity > 0) {
            hasCargo = true;
            printf("- %s: %d tonnes (Purchased at: %d cr/tonne)\n", 
                   playerShip->cargo[i].name, 
                   playerShip->cargo[i].quantity,
                   playerShip->cargo[i].purchasePrice);
        }
    }
    
    if (!hasCargo) {
        printf("Cargo hold is empty.\n");
    }
    
    printf("---------------------------\n");
}

/**
 * Check if the ship has a specific cargo commodity.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to check for
 * @return The quantity of the specified cargo, or 0 if not found
 */
inline int GetCargoQuantity(const PlayerShip* playerShip, const char* cargoName) {
    if (playerShip == NULL || cargoName == NULL) {
        return 0;
    }
    
    int cargoSlot = FindCargoSlot(playerShip, cargoName);
    if (cargoSlot >= 0) {
        return playerShip->cargo[cargoSlot].quantity;
    }
    
    return 0;
}

/**
 * Jettison cargo from the ship (remove cargo without selling it).
 * Useful in emergencies or when cargo is illegal.
 * 
 * @param playerShip Pointer to the PlayerShip structure
 * @param cargoName Name of the cargo/commodity to jettison
 * @param quantity Amount of cargo to jettison (in tonnes)
 * @return true if cargo was successfully jettisoned, false if there was an error
 */
inline bool JettisonCargo(PlayerShip* playerShip, const char* cargoName, int quantity) {
    if (playerShip == NULL || cargoName == NULL || quantity <= 0) {
        return false;
    }
    
    // This simply removes cargo but with a different message
    if (RemoveCargo(playerShip, cargoName, quantity)) {
        printf("Jettisoned %d tonnes of %s into space.\n", quantity, cargoName);
        return true;
    }
    
    return false;
}

#pragma once

#include "elite_player_ship.h"
#include "elite_ship_components.h"
#include "elite_state.h"
#include "platform_compat.h" // For StringCompareIgnoreCase
#include <stdio.h>           // For printf

/**
 * @brief Finds the index of a cargo slot containing a specified cargo item in the player's ship.
 *
 * This function searches through the player's ship cargo slots to find a slot
 * where the cargo name matches the specified name and the quantity is greater than zero.
 * The comparison is done in a case-insensitive manner.
 *
 * @param player_ship_tPointer to the player_ship_tstructure to search within.
 * @param cargoName Name of the cargo item to search for.
 * @return The index of the cargo slot if found; -1 if not found or if input is invalid.
 */
static inline int find_cargo_slot(const player_ship_t *player_ship, const char *cargo_name) {
    if (player_ship == nullptr || cargo_name == nullptr) {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship->cargo[i].quantity > 0 &&
            StringCompareIgnoreCase(player_ship->cargo[i].name, cargo_name) == 0) {
            return i;
        }
    }

    return -1; // Not found
}

/**
 * @brief Finds the index of the first empty cargo slot in the player's ship.
 *
 * This function iterates through the cargo slots of the given player_ship_tand returns
 * the index of the first slot where the quantity is zero, indicating an empty slot.
 *
 * @param player_ship_tPointer to the player_ship_tstructure to search for an empty cargo slot.
 * @return The index of the first empty cargo slot, or -1 if no empty slot is found or if player_ship_tis nullptr.
 */
static inline int find_empty_cargo_slot(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return -1;
    }

    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship->cargo[i].quantity == 0) {
            return i;
        }
    }

    return -1; // No empty slots
}

/**
 * Add cargo to the player's ship. If the cargo already exists, it will increase the quantity.
 * If the cargo doesn't exist, it will be added to the first empty slot.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to add
 * @param quantity Amount of cargo to add (in tonnes)
 * @param purchasePrice Price per tonne (for player's reference)
 * @return 1 if cargo was successfully added, 0 if there was no space
 */
static inline bool add_cargo(player_ship_t *player_ship, const char *cargo_name, int quantity, int purchase_price) {
    if (player_ship == nullptr || cargo_name == nullptr || quantity <= 0) {
        return false;
    }

    // Check if there's enough cargo capacity
    if (player_ship->attributes.currentCargoTons + quantity > player_ship->attributes.cargoCapacityTons) {
        printf("Error: Not enough cargo space. Available: %d tonnes, Needed: %d tonnes\n",
               player_ship->attributes.cargoCapacityTons - player_ship->attributes.currentCargoTons, quantity);
        return false;
    }

    // Check if we already have this cargo type
    int cargo_slot = find_cargo_slot(player_ship, cargo_name);

    if (cargo_slot >= 0) {
        // Cargo already exists, increase quantity
        player_ship->cargo[cargo_slot].quantity += quantity;

        // Update purchase price as the average of the previous and new price
        // This gives a weighted average of purchase prices
        player_ship->cargo[cargo_slot].purchasePrice =
            ((player_ship->cargo[cargo_slot].purchasePrice * (player_ship->cargo[cargo_slot].quantity - quantity)) +
             (purchase_price * quantity)) /
            player_ship->cargo[cargo_slot].quantity;
    } else {
        // Need to find an empty slot for the new cargo type
        cargo_slot = find_empty_cargo_slot(player_ship);

        if (cargo_slot < 0) {
            printf("Error: No available cargo slots. Maximum different cargo types reached.\n");
            return false;
        }

        // Add new cargo type
        int name_length = snprintf(player_ship->cargo[cargo_slot].name, MAX_SHIP_NAME_LENGTH, "%s", cargo_name);
        if (name_length < 0 || name_length >= MAX_SHIP_NAME_LENGTH) {
            printf("Error: Cargo name is too long or could not be formatted.\n");
            return false;
        }
        player_ship->cargo[cargo_slot].quantity = quantity;
        player_ship->cargo[cargo_slot].purchasePrice = purchase_price;
    }

    // Update current cargo weight
    player_ship->attributes.currentCargoTons += quantity;

    printf("Added %d tonnes of %s to cargo hold.\n", quantity, cargo_name);
    return true;
}

/**
 * Remove cargo from the player's ship.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to remove
 * @param quantity Amount of cargo to remove (in tonnes)
 * @return 1 if cargo was successfully removed, 0 if the ship doesn't have that cargo
 */
static inline bool remove_cargo(player_ship_t *player_ship, const char *cargo_name, int quantity) {
    if (player_ship == nullptr || cargo_name == nullptr || quantity <= 0) {
        return false;
    }
    // Find the cargo slot
    int cargo_slot = find_cargo_slot(player_ship, cargo_name);

    if (cargo_slot < 0) {
        printf("\nError: %s not found in cargo hold.", cargo_name);
        return false;
    }

    // Check if we have enough of this cargo
    if (player_ship->cargo[cargo_slot].quantity < quantity) {
        printf("\nError: Not enough %s in cargo hold. Available: %d tonnes, Requested: %d tonnes", cargo_name,
               player_ship->cargo[cargo_slot].quantity, quantity);
        return false;
    }

    // Remove the cargo
    player_ship->cargo[cargo_slot].quantity -= quantity;

    // Update current cargo weight
    player_ship->attributes.currentCargoTons -= quantity;

    // If quantity is now 0, clear the slot
    if (player_ship->cargo[cargo_slot].quantity == 0) {
        // Clear the cargo slot after removing all quantity
        player_ship->cargo[cargo_slot].quantity = 0;
        int name_length = snprintf(player_ship->cargo[cargo_slot].name, MAX_SHIP_NAME_LENGTH, "%s", "Empty");
        if (name_length < 0) {
            player_ship->cargo[cargo_slot].name[0] = '\0';
        } else if (name_length >= MAX_SHIP_NAME_LENGTH) {
            player_ship->cargo[cargo_slot].name[MAX_SHIP_NAME_LENGTH - 1] = '\0';
        }
        player_ship->cargo[cargo_slot].purchasePrice = 0;
    }

    printf("Removed %d tonnes of %s from cargo hold.\n", quantity, cargo_name);
    return true;
}

/**
 * Sell cargo from the player's ship at a trading port and earn credits.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to sell
 * @param quantity Amount of cargo to sell (in tonnes)
 * @param salePrice Price per tonne that will be paid
 * @param externalSync If 1, synchronize with the global state (Cash)
 * @return 1 if cargo was successfully sold, 0 if there was an error
 */
[[maybe_unused]] static inline bool sell_cargo(player_ship_t *player_ship, const char *cargo_name, int quantity,
                                               int sale_price, bool external_sync) {
    if (player_ship == nullptr || cargo_name == nullptr || quantity <= 0 || sale_price < 0) {
        return false;
    }

    // Try to remove the cargo
    if (!remove_cargo(player_ship, cargo_name, quantity)) {
        return false;
    }

    // Calculate the total sale amount
    int total_sale = quantity * sale_price;

    // If we need to sync with the game's global state
    if (external_sync) {
        g_state.Cash += total_sale;
    }

    printf("Sold %d tonnes of %s for %d credits.\n", quantity, cargo_name, total_sale);
    return true;
}

/**
 * Buy cargo for the player's ship from a trading port.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to buy
 * @param quantity Amount of cargo to buy (in tonnes)
 * @param purchasePrice Price per tonne that must be paid
 * @param externalSync If 1, synchronize with the global state (Cash)
 * @return 1 if cargo was successfully purchased, 0 if there was an error
 */
[[maybe_unused]] static inline bool buy_cargo(player_ship_t *player_ship, const char *cargo_name, int quantity,
                                              int purchase_price, bool external_sync) {
    if (player_ship == nullptr || cargo_name == nullptr || quantity <= 0 || purchase_price < 0) {
        return false;
    }

    // Calculate the total cost
    int total_cost = quantity * purchase_price;

    // If we need to sync with the game's global state, check if we have enough cash
    if (external_sync) {
        if (g_state.Cash < total_cost) {
            printf("Error: Not enough credits. Available: %d, Required: %d\n", g_state.Cash, total_cost);
            return false;
        }
    }

    // Try to add the cargo
    if (!add_cargo(player_ship, cargo_name, quantity, purchase_price)) {
        return false;
    }

    // If everything successful and we need to sync, deduct the cash
    if (external_sync) {
        g_state.Cash -= total_cost;
    }

    printf("Purchased %d tonnes of %s for %d credits.\n", quantity, cargo_name, total_cost);
    return true;
}

/**
 * List all cargo items in the player's cargo hold.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 */
[[maybe_unused]] static inline void list_cargo(const player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return;
    }

    printf("\n--- Cargo Hold (%d/%d tonnes) ---\n", player_ship->attributes.currentCargoTons,
           player_ship->attributes.cargoCapacityTons);

    bool has_cargo = false;
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship->cargo[i].quantity > 0) {
            has_cargo = true;
            printf("- %s: %d tonnes (Purchased at: %d cr/tonne)\n", player_ship->cargo[i].name,
                   player_ship->cargo[i].quantity, player_ship->cargo[i].purchasePrice);
        }
    }

    if (!has_cargo) {
        printf("Cargo hold is empty.\n");
    }

    printf("---------------------------\n");
}

/**
 * Check if the ship has a specific cargo commodity.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to check for
 * @return The quantity of the specified cargo, or 0 if not found
 */
[[maybe_unused]] static inline int get_cargo_quantity(const player_ship_t *player_ship, const char *cargo_name) {
    if (player_ship == nullptr || cargo_name == nullptr) {
        return 0;
    }

    int cargo_slot = find_cargo_slot(player_ship, cargo_name);
    if (cargo_slot >= 0) {
        return player_ship->cargo[cargo_slot].quantity;
    }

    return 0;
}

/**
 * Jettison cargo from the ship (remove cargo without selling it).
 * Useful in emergencies or when cargo is illegal.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @param cargoName Name of the cargo/commodity to jettison
 * @param quantity Amount of cargo to jettison (in tonnes)
 * @return 1 if cargo was successfully jettisoned, 0 if there was an error
 */
[[maybe_unused]] static inline bool jettison_cargo(player_ship_t *player_ship, const char *cargo_name, int quantity) {
    if (player_ship == nullptr || cargo_name == nullptr || quantity <= 0) {
        return false;
    }

    // This simply removes cargo but with a different message
    if (remove_cargo(player_ship, cargo_name, quantity)) {
        printf("\nJettisoned %d tonnes of %s into space.", quantity, cargo_name);
        return true;
    }

    return false;
}

/**
 * Jettison all cargo from the ship (remove all cargo without selling it).
 * Useful in emergencies such as being chased by authorities or in combat situations.
 *
 * @param player_ship_tPointer to the player_ship_tstructure
 * @return 1 if cargo was successfully jettisoned, 0 if there was an error
 */
[[maybe_unused]] static inline bool jettison_all_cargo(player_ship_t *player_ship) {
    if (player_ship == nullptr) {
        return false;
    }

    // Check if the ship has any cargo at all
    if (player_ship->attributes.currentCargoTons <= 0) {
        printf("\nNo cargo to jettison.");
        return false;
    }

    int total_jettisoned = 0;

    // Iterate through all cargo slots
    for (int i = 0; i < MAX_CARGO_SLOTS; ++i) {
        if (player_ship->cargo[i].quantity > 0) {
            int quantity = player_ship->cargo[i].quantity;
            total_jettisoned += quantity;

            printf("\nJettisoned %d tonnes of %s into space.", quantity, player_ship->cargo[i].name);

            // Clear the cargo slot
            player_ship->cargo[i].quantity = 0;
            (void)snprintf(player_ship->cargo[i].name, MAX_SHIP_NAME_LENGTH, "%s", "Empty");
            player_ship->cargo[i].purchasePrice = 0;
        }
    }

    // Reset current cargo weight
    player_ship->attributes.currentCargoTons = 0;

    // Print summary
    printf("\nAll cargo jettisoned: %d tonnes total.", total_jettisoned);

    return true;
}

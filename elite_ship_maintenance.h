#pragma once

#include "elite_player_ship.h"
#include "elite_state.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Consumes a specified amount of fuel from the ship
 * Used for both hyperspace jumps and in-system travel
 *
 * @param fuelAmount Amount of fuel to consume in tenths of LY for jumps, or liters for local travel
 * @param isLocalTravel If 1, converts from liters to LY units for in-system travel
 * @return 1 if consumption was successful, 0 if insufficient fuel
 */
[[maybe_unused]] static inline bool consume_fuel(double fuel_amount, bool is_local_travel) {
    // Debug output can be uncommented for testing
    // printf("\nConsumeFuel: initial Fuel=%d, amount=%.3f, isLocal=%d", g_state.Fuel, fuelAmount, isLocalTravel);

    double fuel_in_tenths_ly = fuel_amount;

    // For local travel, the fuel amount is in liters
    // Convert from liters to tenths of LY (10 liters = 0.1 LY)
    if (is_local_travel) {
        fuel_in_tenths_ly = fuel_amount / 10.0;
    }

    // Round to get integer value with appropriate precision
    double rounded_fuel = ceil(fuel_in_tenths_ly);
    int fuel_to_consume = (int)rounded_fuel;

    // Check if we have enough fuel
    if (g_state.Fuel < fuel_to_consume) {
        return false;
    }

    // Consume the fuel from the global variable
    g_state.Fuel -= fuel_to_consume;

    // Also update ship's internal fuel representation
    if (g_state.PlayerShipPtr != nullptr) {
        // Update ship's fuel in liters (1 tenth of LY = 10 liters)
        // Ensure we never go below zero
        double fuel_liters_to_consume = (int)is_local_travel ? fuel_amount : fuel_amount * 10.0;
        g_state.PlayerShipPtr->attributes.fuelLiters =
            fmax(g_state.PlayerShipPtr->attributes.fuelLiters - fuel_liters_to_consume, 0.0);

        // Make sure global Fuel and ship's fuel are in sync
        // This accounts for any rounding errors
        g_state.Fuel = (uint16_t)(g_state.PlayerShipPtr->attributes.fuelLiters / 10.0);
    }

    return true;
}

/**
 * Interface to the existing RefuelShip function in elite_ship_types.h
 * This function maintains compatibility with the updated system
 *
 * @param amount Amount of fuel to add in LY units
 * @param useCash If 1, deducts cash; if 0, attempts to use fuel scoops
 * @return 1 if refueling was successful, 0 otherwise
 */
[[maybe_unused]] static inline bool ship_refuel(double amount, bool use_cash) {
    if (g_state.PlayerShipPtr == nullptr) {
        return false;
    }

    // The isEmergency parameter was part of the original signature but not used in the call to RefuelShip.
    // RefuelShip from elite_ship_types.h takes: PlayerShip*, amount, useFuelScoops (1 if !useCash),
    // allowEmergencyRefuel (always 1 here)
    double result = refuel_ship(g_state.PlayerShipPtr, amount, (!use_cash) != 0, true);
    return result > 0.0;
}

/**
 * Uses fuel scoops to collect fuel from a suitable source
 * Usually used for fuel scooping from stars or gas giants
 *
 * @param amount Amount to try scooping in LY units
 * @return 1 if scooping was successful
 */
[[maybe_unused]] static inline bool use_fuel_scoops(double amount) {
    if (g_state.PlayerShipPtr == nullptr) {
        return false;
    }

    // Call the ship-specific RefuelShip with appropriate parameters
    refuel_ship(g_state.PlayerShipPtr, amount, true, true);
    return true;
}

/**
 * Interface to the existing RepairHull function in elite_ship_types.h
 *
 * @param repairAmount Amount of hull strength to repair
 * @param useCash If 1, deducts cash; if 0, attempts emergency repair
 * @return 1 if repair was successful, 0 otherwise
 */
[[maybe_unused]] static inline bool ship_repair(int repair_amount, bool use_cash) {
    if (g_state.PlayerShipPtr == nullptr) {
        return false;
    }

    // Standard cost per hull point is 5 credits
    int cost_per_point = 5;

    // If not using cash, we use the emergency repair mode which is
    // handled differently by the underlying function
    int result = repair_hull(g_state.PlayerShipPtr, repair_amount, (int)use_cash ? cost_per_point : 0, true);

    return result > 0;
}

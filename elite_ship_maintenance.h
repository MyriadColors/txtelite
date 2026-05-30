#pragma once

#include "elite_player_ship.h"
#include "elite_equipment_constants.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/**
 * Consumes a specified amount of fuel from the ship
 * Used for both hyperspace jumps and in-system travel
 *
 * @param fuelAmount Amount of fuel to consume in tenths of LY for jumps, or liters for local travel
 * @param isLocalTravel If 1, converts from liters to LY units for in-system travel
 * @return 1 if consumption was successful, 0 if insufficient fuel
 */
static inline bool ConsumeFuel(double fuelAmount, bool isLocalTravel)
{
    // Debug output can be uncommented for testing
    // printf("\nConsumeFuel: initial Fuel=%d, amount=%.3f, isLocal=%d", g_state.Fuel, fuelAmount, isLocalTravel);
    
    double fuelInTenthsLY = fuelAmount;
    
    // For local travel, the fuel amount is in liters
    // Convert from liters to tenths of LY (10 liters = 0.1 LY)
    if (isLocalTravel) {
        fuelInTenthsLY = fuelAmount / 10.0;
    }
    
    // Round to get integer value with appropriate precision
    int fuelToConsume = (int)ceil(fuelInTenthsLY);
    
    // Check if we have enough fuel
    if (g_state.Fuel < fuelToConsume) {
        return 0;
    }
    
    // Consume the fuel from the global variable
    g_state.Fuel -= fuelToConsume;
    
    // Also update ship's internal fuel representation
    if (g_state.PlayerShipPtr != NULL) {
        // Update ship's fuel in liters (1 tenth of LY = 10 liters)
        // Ensure we never go below zero
        double fuelLitersToConsume = isLocalTravel ? fuelAmount : fuelAmount * 10.0;
        g_state.PlayerShipPtr->attributes.fuelLiters = fmax(g_state.PlayerShipPtr->attributes.fuelLiters - fuelLitersToConsume, 0.0);
        
        // Make sure global Fuel and ship's fuel are in sync
        // This accounts for any rounding errors
        g_state.Fuel = (uint16_t)(g_state.PlayerShipPtr->attributes.fuelLiters / 10.0);
    }
    
    return 1;
}

/**
 * Interface to the existing RefuelShip function in elite_ship_types.h
 * This function maintains compatibility with the updated system
 *
 * @param amount Amount of fuel to add in LY units
 * @param useCash If 1, deducts cash; if 0, attempts to use fuel scoops
 * @return 1 if refueling was successful, 0 otherwise
 */
static inline bool ShipRefuel(double amount, bool useCash)
{
    if (g_state.PlayerShipPtr == NULL) {
        return 0;
    }
    
    // The isEmergency parameter was part of the original signature but not used in the call to RefuelShip.
    // RefuelShip from elite_ship_types.h takes: PlayerShip*, amount, useFuelScoops (1 if !useCash), allowEmergencyRefuel (always 1 here)
    float result = RefuelShip(g_state.PlayerShipPtr, (float)amount, !useCash, 1);
    return result > 0.0f;
}

/**
 * Uses fuel scoops to collect fuel from a suitable source
 * Usually used for fuel scooping from stars or gas giants
 * 
 * @param amount Amount to try scooping in LY units
 * @return 1 if scooping was successful
 */
static inline bool UseFuelScoops(double amount)
{
    if (g_state.PlayerShipPtr == NULL) {
        return 0;
    }
    
    // Call the ship-specific RefuelShip with appropriate parameters
    RefuelShip(g_state.PlayerShipPtr, (float)amount, 1, 1);
    return 1;
}

/**
 * Interface to the existing RepairHull function in elite_ship_types.h
 * 
 * @param repairAmount Amount of hull strength to repair
 * @param useCash If 1, deducts cash; if 0, attempts emergency repair 
 * @return 1 if repair was successful, 0 otherwise
 */
static inline bool ShipRepair(int repairAmount, bool useCash)
{
    if (g_state.PlayerShipPtr == NULL) {
        return 0;
    }
    
    // Standard cost per hull point is 5 credits
    int costPerPoint = 5;
    
    // If not using cash, we use the emergency repair mode which is
    // handled differently by the underlying function
    int result = RepairHull(g_state.PlayerShipPtr, repairAmount, 
                           useCash ? costPerPoint : 0, 1);
                           
    return result > 0;
}


/**
 * Gets the fuel cost per unit based on ship type
 * This is an inline declaration of the function from txtelite.c
 *
 * @return Cost of fuel unit (for 0.1 LY of travel)
 */
extern int GetFuelCost(void);

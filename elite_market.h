#pragma once

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_utils.h"  // For minimum_value
#include <math.h>         // For floor (used in execute_buy_order)

// This macro was originally with the Commodities array definition
#define POLITICALLY_CORRECT 0
/* Set to 1 for NES-sanitised trade goods */

// UnitNames array, static within this header
static char UnitNames[][5] ={"t","kg","g"};

// Commodities array, static within this header
// Defines NUM_STANDARD_COMMODITIES (10) items
static TradeGood Commodities[] = {
    {0x13, -0x02, 0x06, 0x01, 0, "Food        "},
    {0x14, -0x01, 0x0A, 0x03, 0, "Textiles    "},
    {0x41, -0x03, 0x02, 0x07, 0, "Radioactives"},
#if POLITICALLY_CORRECT
    {0x28, -0x05, 0xE2, 0x1F, 0, "Robot Slaves"},
    {0x53, -0x05, 0xFB, 0x0F, 0, "Beverages   "},
#else
    {0x28, -0x05, 0xE2, 0x1F, 0, "Slaves      "},
    {0x53, -0x05, 0xFB, 0x0F, 0, "Liquor/Wines"},
#endif
    {0xC4, +0x08, 0x36, 0x03, 0, "Luxuries    "},
#if POLITICALLY_CORRECT
    {0xEB, +0x1D, 0x08, 0x78, 0, "Rare Species"},
#else
    {0xEB, +0x1D, 0x08, 0x78, 0, "Narcotics   "},
#endif
    {0x9A, +0x0E, 0x38, 0x03, 0, "Computers   "},
    {0x75, +0x06, 0x28, 0x07, 0, "Machinery   "},
    {0x4E, +0x01, 0x11, 0x1F, 0, "Alloys      "},
};

// Initializes the global tradnames array.
// tradnames is char tradnames[LAST_TRADE + 1][MAX_LEN];
// Copies names for the first NUM_STANDARD_COMMODITIES.
// Clears remaining entries up to LAST_TRADE.
static inline void init_tradnames(void) {
    uint16_t i;
    // Copy names from the Commodities array
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        if (i < (sizeof(Commodities) / sizeof(Commodities[0]))) {
            strncpy(tradnames[i], Commodities[i].name, MAX_LEN - 1);
            tradnames[i][MAX_LEN - 1] = '\0'; // Ensure null termination
        } else {
            tradnames[i][0] = '\0';
        }
    }
    // Initialize the remaining part of tradnames up to LAST_TRADE.
    for (; i <= LAST_TRADE; i++) {
        if (i < (LAST_TRADE + 1)) { 
            tradnames[i][0] = '\0';
        }
    }
}

// Generates market data for a given planet system and fluctuation.
// MarketType arrays are sized COMMODITY_ARRAY_SIZE (ALIEN_ITEMS_IDX + 1).
static inline MarketType generate_market(uint16_t fluctuation, struct PlanSys planetSystem) {
    MarketType market;
    uint16_t i;

    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        int32_t q;
        int32_t product = (planetSystem.economy) * (Commodities[i].gradient);
        int32_t changing = fluctuation & (Commodities[i].maskByte);
        q = (Commodities[i].baseQuant) + changing - product;
        q = q & 0xFF;
        if (q & 0x80) { q = 0; } 
        market.quantity[i] = (uint16_t)(q & 0x3F); 

        q = (Commodities[i].basePrice) + changing + product;
        q = q & 0xFF;
        market.price[i] = (uint16_t)(q * 4);
    }

    for (i = NUM_STANDARD_COMMODITIES; i <= LAST_TRADE; i++) {
        market.quantity[i] = 0;
        market.price[i] = 0;
    }

    market.quantity[ALIEN_ITEMS_IDX] = 0; 
    market.price[ALIEN_ITEMS_IDX] = 0;    

    return market;
}

// Displays the market information.
static inline void display_market_info(MarketType marketData) {
    uint16_t i;
    printf("ITEM          PRICE  QTY UNIT CARGO"); 

    // Only show the actual defined commodities (NUM_STANDARD_COMMODITIES)
    // This matches the original game behavior more closely
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        printf("\n");
        printf("%-12s", Commodities[i].name); 
        printf("   %5.1f", ((float)(marketData.price[i]) / 10)); 
        printf("   %3u", marketData.quantity[i]); 
        printf(" %-3s", UnitNames[Commodities[i].units]); 
        printf("   %3u", ShipHold[i]); 
    }
    printf("\n"); 
}

// Executes a buy order for a given item and amount.
static inline uint16_t execute_buy_order(uint16_t itemIndex, uint16_t amount) {
    uint16_t t;
    if (Cash < 0) { t = 0; }
    else {
        if (itemIndex >= COMMODITY_ARRAY_SIZE) return 0; 

        t = minimum_value(LocalMarket.quantity[itemIndex], amount);

        if (itemIndex < NUM_STANDARD_COMMODITIES) {
            if ((Commodities[itemIndex].units) == TONNES_UNIT) {
                t = minimum_value(HoldSpace, t);
            }
        }
        
        if (LocalMarket.price[itemIndex] > 0) {
            t = minimum_value(t, (uint16_t)floor((double)Cash / LocalMarket.price[itemIndex]));
        } else if (Cash > 0 && LocalMarket.quantity[itemIndex] > 0 && LocalMarket.price[itemIndex] == 0) {
            // Free item, t is already min(available, requested)
        } else { 
            t = 0;
        }
    }

    if (itemIndex >= COMMODITY_ARRAY_SIZE) return 0;

    ShipHold[itemIndex] += t;
    LocalMarket.quantity[itemIndex] -= t;
    Cash -= (int32_t)t * (LocalMarket.price[itemIndex]); 

    if (itemIndex < NUM_STANDARD_COMMODITIES) {
        if ((Commodities[itemIndex].units) == TONNES_UNIT) {
            HoldSpace -= t;
        }
    }
    return t;
}

// Executes a sell order for a given item and amount.
static inline uint16_t execute_sell_order(uint16_t itemIndex, uint16_t amount) {
    if (itemIndex >= COMMODITY_ARRAY_SIZE) return 0; 

    uint16_t t = minimum_value(ShipHold[itemIndex], amount);

    ShipHold[itemIndex] -= t;
    LocalMarket.quantity[itemIndex] += t; 
    Cash += (int32_t)t * (LocalMarket.price[itemIndex]);

    if (itemIndex < NUM_STANDARD_COMMODITIES) {
        if ((Commodities[itemIndex].units) == TONNES_UNIT) {
            HoldSpace += t;
        }
    }
    return t;
}

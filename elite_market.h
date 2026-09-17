#pragma once

#include "elite_ship_components.h"
#include "elite_state.h"     // Unified header for constants, structures, and globals
#include "elite_utils.h"     // For minimum_value
#include "platform_compat.h" // For StringCompareIgnoreCase
#include <stdint.h>
#include <stdio.h>

// Define constants for market modifiers if not already defined elsewhere
// These represent a 25% change.
#define PRICE_DECREASE_FACTOR 0.75f
#define PRICE_INCREASE_FACTOR 1.25f
#define QUANTITY_DECREASE_FACTOR 0.75f
#define QUANTITY_INCREASE_FACTOR 1.25f
#define NO_CHANGE_FACTOR 1.0F

// Enum for Station Specializations
typedef enum station_specialization_t {
    STATION_SPECIALIZATION_BALANCED = 0,
    STATION_SPECIALIZATION_INDUSTRIAL = 1,
    STATION_SPECIALIZATION_AGRICULTURAL = 2,
    STATION_SPECIALIZATION_MINING = 3,
    NUM_STATION_SPECIALIZATIONS = 4 // Keep this last for array sizing
} station_specialization_t;

// Enum for planet_tMarket Types (using "MarketType" to distinguish from general planet properties if any)
typedef enum planet_market_type_t {
    PLANET_MARKET_TYPE_ROCKY_AIRLESS = 0,
    PLANET_MARKET_TYPE_TERRESTRIAL = 1,
    PLANET_MARKET_TYPE_GAS_GIANT = 2,
    PLANET_MARKET_TYPE_ICE_GIANT_WORLD = 3,
    NUM_PLANET_MARKET_TYPES = 4 // Keep this last for array sizing
} planet_market_type_t;

// Structure to hold market modifiers for a single commodity
typedef struct market_modifier_t {
    float priceFactor; // Added priceFactor
    float quantityFactor;
} market_modifier_t;

// This macro was originally with the Commodities array definition
#define POLITICALLY_CORRECT 0
/* Set to 1 for NES-sanitised trade goods */

// UnitNames array, static within this header
static char g_unit_names[][5] = {"t", "kg", "g"};

// Commodities array, static within this header
// Defines NUM_STANDARD_COMMODITIES (10) items
static trade_good_t g_commodities[] = {
    {0x13, -0x02, 0x06, 0x01, 0, "Food        "}, {0x14, -0x01, 0x0A, 0x03, 0, "Textiles    "},
    {0x41, -0x03, 0x02, 0x07, 0, "Radioactives"},
#if POLITICALLY_CORRECT
    {0x28, -0x05, 0xE2, 0x1F, 0, "Robot Slaves"}, {0x53, -0x05, 0xFB, 0x0F, 0, "Beverages   "},
#else
    {0x28, -0x05, 0xE2, 0x1F, 0, "Slaves      "}, {0x53, -0x05, 0xFB, 0x0F, 0, "Liquor/Wines"},
#endif
    {0xC4, +0x08, 0x36, 0x03, 0, "Luxuries    "},
#if POLITICALLY_CORRECT
    {0xEB, +0x1D, 0x08, 0x78, 0, "Rare Species"},
#else
    {0xEB, +0x1D, 0x08, 0x78, 0, "Narcotics   "},
#endif
    {0x9A, +0x0E, 0x38, 0x03, 0, "Computers   "}, {0x75, +0x06, 0x28, 0x07, 0, "Machinery   "},
    {0x4E, +0x01, 0x11, 0x1F, 0, "Alloys      "},
};

// Market Modifiers for Station Specializations
// Indexed by [station_specialization_t][CommodityIndex]
// Commodity Indices: 0:Food, 1:Textiles, 2:Radioactives, 3:Slaves, 4:Liquor, 5:Luxuries, 6:Narcotics, 7:Computers,
// 8:Machinery, 9:Alloys
[[maybe_unused]] static market_modifier_t
    g_station_specialization_modifiers[NUM_STATION_SPECIALIZATIONS][NUM_STANDARD_COMMODITIES] = {
        // STATION_SPECIALIZATION_BALANCED (0) - Minor or no strong modifications
        {{NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}},
        // STATION_SPECIALIZATION_INDUSTRIAL (1)
        // Produces: Machinery (8), Alloys (9)
        // Consumes: Food (0), Textiles (1), Radioactives (2)
        {{PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
         {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}},
        // STATION_SPECIALIZATION_AGRICULTURAL (2)
        // Produces: Food (0), Textiles (1)
        // Consumes: Machinery (8), Luxuries (5)
        {{PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
         {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}},
        // STATION_SPECIALIZATION_MINING (3)
        // Produces: Radioactives (2), Alloys (9)
        // Consumes: Food (0), Machinery (8)
        {{PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
         {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
         {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}}};

// Market Modifiers for planet_tTypes
// Indexed by [planet_market_type_t][CommodityIndex]
[[maybe_unused]] static market_modifier_t g_planet_type_modifiers[NUM_PLANET_MARKET_TYPES][NUM_STANDARD_COMMODITIES] = {
    // PLANET_MARKET_TYPE_ROCKY_AIRLESS (0)
    // Produces: Radioactives (2), Alloys (9)
    // Consumes: Food (0), Textiles (1)
    {{PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR}},
    // PLANET_MARKET_TYPE_TERRESTRIAL (1)
    // Produces: Food (0), Textiles (1)
    // Consumes: Machinery (8), Computers (7)
    {{PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
     {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}},
    // PLANET_MARKET_TYPE_GAS_GIANT (2)
    // Produces: Radioactives (2) (e.g. fuel components)
    // Consumes: Food (0), Machinery (8), Computers (7)
    {{PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}},
    // PLANET_MARKET_TYPE_ICE_GIANT_WORLD (3)
    // Produces: Liquor/Wines (4)
    // Consumes: Food (0), Machinery (8), Luxuries (5)
    {{PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_DECREASE_FACTOR, QUANTITY_INCREASE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR},
     {PRICE_INCREASE_FACTOR, QUANTITY_DECREASE_FACTOR},
     {NO_CHANGE_FACTOR, NO_CHANGE_FACTOR}}};

// Initializes the global tradnames array.
// Copies names for the first NUM_STANDARD_COMMODITIES.
// Clears remaining entries up to LAST_TRADE.
[[maybe_unused]] static inline void init_tradnames(void) {
    uint16_t i;
    // Copy names from the Commodities array
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        if (i < (sizeof(g_commodities) / sizeof(g_commodities[0]))) {
            safe_snprintf(g_state.tradnames[i], MAX_LEN, "%s", g_commodities[i].name);
        } else {
            g_state.tradnames[i][0] = '\0';
        }
    }
    // Initialize the remaining part of tradnames up to LAST_TRADE.
    for (; i <= LAST_TRADE; i++) {
        if (i < (LAST_TRADE + 1)) {
            g_state.tradnames[i][0] = '\0';
        }
    }
}

// Generates market data for a given planet system and fluctuation.
// MarketType arrays are sized COMMODITY_ARRAY_SIZE (ALIEN_ITEMS_IDX + 1).
[[maybe_unused]] static inline market_type_t generate_market(uint16_t fluctuation, struct plan_sys_t planet_system) {
    market_type_t market;
    uint16_t i;

    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        int32_t q;
        int32_t product = (planet_system.economy) * (g_commodities[i].gradient);
        int32_t changing = fluctuation & (g_commodities[i].maskByte);
        q = (g_commodities[i].baseQuant) + changing - product;
        q = q & 0xFF;
        if (q & 0x80) {
            q = 0;
        }
        market.quantity[i] = (uint16_t)(q & 0x3F);

        q = (g_commodities[i].basePrice) + changing + product;
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
[[maybe_unused]] static inline void display_market_info(market_type_t market_data) {
    uint16_t i;
    printf("ITEM          PRICE  QTY UNIT CARGO");

    // Only show the actual defined commodities (NUM_STANDARD_COMMODITIES)
    // This matches the original game behavior more closely
    for (i = 0; i < NUM_STANDARD_COMMODITIES; i++) {
        uint16_t cargo_qty = 0;
        if (g_state.PlayerShipPtr != nullptr) {
            for (int j = 0; j < MAX_CARGO_SLOTS; j++) {
                if (g_state.PlayerShipPtr->cargo[j].quantity > 0 &&
                    StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[j].name, g_commodities[i].name) == 0) {
                    cargo_qty += g_state.PlayerShipPtr->cargo[j].quantity;
                }
            }
        }

        printf("\n");
        printf("%-12s", g_commodities[i].name);
        printf("   %5.1f", ((double)market_data.price[i] / 10.0));
        printf("   %3u", market_data.quantity[i]);
        printf(" %-3s", g_unit_names[g_commodities[i].units]);
        printf("   %3u", cargo_qty);
    }
    printf("\n");
}

// Executes a buy order for a given item and amount.
[[maybe_unused]] static inline uint16_t buy_order_quantity(uint16_t item_index, uint16_t amount) {
    if (g_state.Cash < 0 || item_index >= COMMODITY_ARRAY_SIZE) {
        return 0;
    }

    uint16_t quantity = minimum_value(g_state.LocalMarket.quantity[item_index], amount);
    if (item_index < NUM_STANDARD_COMMODITIES && g_commodities[item_index].units == TONNES_UNIT) {
        uint16_t hold_space = 0;
        if (g_state.PlayerShipPtr != nullptr) {
            hold_space = (uint8_t)(g_state.PlayerShipPtr->attributes.cargoCapacityTons -
                                   g_state.PlayerShipPtr->attributes.currentCargoTons);
        }
        quantity = minimum_value(hold_space, quantity);
    }

    if (g_state.LocalMarket.price[item_index] > 0) {
        return minimum_value(quantity, (uint16_t)((double)g_state.Cash / g_state.LocalMarket.price[item_index]));
    }
    if (g_state.Cash > 0 && g_state.LocalMarket.quantity[item_index] > 0) {
        return quantity;
    }
    return 0;
}

[[maybe_unused]] static inline int add_buy_order_cargo(uint16_t item_index, uint16_t quantity) {
    if (g_state.PlayerShipPtr == nullptr) {
        return 1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
        if (g_state.PlayerShipPtr->cargo[i].quantity > 0 &&
            StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[i].name, g_commodities[item_index].name) == 0) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
            if (g_state.PlayerShipPtr->cargo[i].quantity == 0) {
                safe_snprintf(g_state.PlayerShipPtr->cargo[i].name, MAX_SHIP_NAME_LENGTH, "%s",
                              g_commodities[item_index].name);
                slot = i;
                g_state.PlayerShipPtr->cargo[slot].purchasePrice = g_state.LocalMarket.price[item_index] / 10;
                break;
            }
        }
    }
    if (slot == -1) {
        return 0;
    }

    g_state.PlayerShipPtr->cargo[slot].quantity += quantity;
    if (g_commodities[item_index].units == TONNES_UNIT) {
        g_state.PlayerShipPtr->attributes.currentCargoTons += quantity;
    }
    return 1;
}

[[maybe_unused]] static inline uint16_t execute_buy_order(uint16_t item_index, uint16_t amount) {
    uint16_t t = buy_order_quantity(item_index, amount);
    if (t == 0) {
        return 0;
    }
    if (!add_buy_order_cargo(item_index, t)) {
        return 0;
    }

    g_state.LocalMarket.quantity[item_index] -= t;
    g_state.Cash -= (int32_t)t * (g_state.LocalMarket.price[item_index]);

    return t;
}

// Executes a sell order for a given item and amount.
[[maybe_unused]] static inline uint16_t execute_sell_order(uint16_t item_index, uint16_t amount) {
    if (item_index >= COMMODITY_ARRAY_SIZE || g_state.PlayerShipPtr == nullptr) {
        return 0;
    }

    uint16_t cargo_qty = 0;
    int slot = -1;
    for (int i = 0; i < MAX_CARGO_SLOTS; i++) {
        if (g_state.PlayerShipPtr->cargo[i].quantity > 0 &&
            StringCompareIgnoreCase(g_state.PlayerShipPtr->cargo[i].name, g_commodities[item_index].name) == 0) {
            cargo_qty = (uint8_t)g_state.PlayerShipPtr->cargo[i].quantity;
            slot = i;
            break;
        }
    }

    uint16_t t = minimum_value(cargo_qty, amount);
    if (t == 0) {
        return 0;
    }

    g_state.PlayerShipPtr->cargo[slot].quantity -= t;
    if (g_state.PlayerShipPtr->cargo[slot].quantity == 0) {
        safe_snprintf(g_state.PlayerShipPtr->cargo[slot].name, MAX_SHIP_NAME_LENGTH, "Empty");
    }

    if (g_commodities[item_index].units == TONNES_UNIT) {
        g_state.PlayerShipPtr->attributes.currentCargoTons -= t;
    }

    g_state.LocalMarket.quantity[item_index] += t;
    g_state.Cash += (int32_t)t * (g_state.LocalMarket.price[item_index]);

    return t;
}

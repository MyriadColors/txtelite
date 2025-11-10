#pragma once

#define KGRN  "\x1b[32m"
#define KRED  "\x1b[31m"
#define KNRM  "\x1b[0m"

// Include headers for necessary typedefs and forward declarations
#include "elite_star_system.h" // For Star, Planet, Station, etc.

// Include the rest of the headers
#include "elite_market.h" // For execute_buy_order, execute_sell_order, display_market_info
#include "elite_navigation.h" // For distance, find_matching_system_name, execute_jump_to_planet
#include "elite_planet_info.h" // For print_system_info (and goat_soup)
#include "elite_player_state.h" // For calculate_fuel_purchase, display_ship_status_brief
#include "elite_save.h"           // For save_game, load_game
#include "elite_ship_cargo.h"     // For cargo management functions
#include "elite_ship_inventory.h" // For inventory management functions
#include "elite_ship_maintenance.h" // For ship maintenance functions including ConsumeFuel
#include "elite_ship_trading.h"  // For ship trading commands
#include "elite_ship_types.h"    // For ship status display functions
#include "elite_ship_upgrades.h" // For ship upgrades
#include "elite_state.h" // Unified header for constants, structures, and globals
#include "platform_compat.h" // For cross-platform file operations and StringCompareIgnoreCase
#include <ctype.h>           // For toupper, tolower
#include <math.h>            // For floor, fabs
#include <stdlib.h>          // For atoi, atof
#include <string.h>          // For string operations
#include <time.h>            // For time functions

// Command help structure
typedef struct {
    const char *command;
    const char *aliases;  // Space-separated list of aliases or NULL
    const char *short_description;
    const char *long_description;
    const char *category;
    bool (*is_available)(void); // Function to check if the command is available
} CommandHelp;

// Availability checks for commands
static inline bool is_always_available(void) { return 1; }
static inline bool is_docked(void) { return PlayerLocationType == 10; }
static inline bool is_not_docked(void) { return PlayerLocationType != 10; }
static inline bool is_at_planet(void) { return PlayerNavState.currentLocationType == CELESTIAL_PLANET; }
static inline bool is_at_station(void) { return PlayerNavState.currentLocationType == CELESTIAL_STATION; }

// Command help data
static const CommandHelp command_help[] = {
    // TRADING COMMANDS
    {"buy", "b", "Purchase goods from the market", 
     "BUY <good> <amount> - Purchase goods from the market\n" "  <good>   - Type of trade good (e.g., Food, Computers)\n" "  <amount> - Quantity to buy (default: 1)\n" "  Example: buy Food 5\n" "  Note: You must be docked at a station with a market to buy goods.", "TRADING COMMANDS", is_docked},
    
    {"sell", "s", "Sell goods to the market", 
     "SELL <good> <amount> - Sell goods to the market\n" "  <good>   - Type of trade good (e.g., Food, Computers)\n" "  <amount> - Quantity to sell (default: 1)\n" "  Example: sell Computers 3\n" "  Note: You must be docked at a station with a market to sell goods.", "TRADING COMMANDS", is_docked},

    {"mkt", "m", "Display market information", 
     "MKT - Display market information\n" "  Shows current market prices, cash, fuel level, and cargo status.\n" "  No parameters required.\n" "  Note: Market prices vary between systems based on economy type.", "TRADING COMMANDS", is_docked},

    {"compare", NULL, "Compare markets across different stations in the system", 
     "COMPARE - Compare markets across different stations in the system\n" "  Shows price differences and profit opportunities between stations.\n" "  Lists all stations in the system with their distance from you.\n" "  Highlights best commodities to buy or sell at each station.\n" "  Shows estimated travel times to other stations.\n" "  Note: You must be docked at a station to use this command.", "TRADING COMMANDS", is_docked},

    // INTERSTELLAR NAVIGATION
    {"jump", "j", "Jump to another star system", 
     "JUMP <planetname> - Jump to another star system\n" "  <planetname> - Name of the destination system\n" "  Example: jump Lave\n" "  Note: Requires fuel equal to the distance in light years.\n" "        Use 'local' to see systems within jump range.", "INTERSTELLAR NAVIGATION", is_not_docked},

    {"local", "l", "List star systems within jump range", 
     "LOCAL - List star systems within jump range\n" "  Systems marked with * are within current fuel range.\n" "  Systems marked with - are within maximum fuel capacity but require refueling.\n" "  Distances are shown in light years (LY).", "INTERSTELLAR NAVIGATION", is_not_docked},

    {"galhyp", "g", "Perform a galactic hyperspace jump", 
     "GALHYP - Perform a galactic hyperspace jump\n" "  Jumps to the next galaxy (1-8).\n" "  No fuel is required for this special jump.", "INTERSTELLAR NAVIGATION", is_not_docked},

    {"info", "i", "Display information about a system", 
     "INFO <planetname> - Display information about a system\n" "  <planetname> - Name of the system to get information about\n" "  Example: info Lave\n" "  Shows economy, government, tech level, and other system details.", "INTERSTELLAR NAVIGATION", is_always_available},

    {"fuel", "f", "Purchase fuel for your ship", 
     "FUEL <amount> - Purchase fuel for your ship\n" "  <amount> - Amount of fuel to buy in light years\n" "  Example: fuel 2.5\n" "  Note: You must be docked at a station to buy fuel.\n" "        Fuel costs %.1f credits per 0.1 LY unit for your current ship", "INTERSTELLAR NAVIGATION", is_docked},

    // STAR SYSTEM NAVIGATION
    {"system", "sys", "Displays detailed information about the current star system", 
     "SYSTEM - Displays detailed information about the current star system\n" "  Shows all celestial bodies, stations, their locations, and travel times.\n" "  Note: This command (formerly also available as 'scan') scans the system\n" "        for points of interest and costs 1 minute of game time.", "STAR SYSTEM NAVIGATION", is_not_docked},

    {"travel", "t", "Travel within the current star system", 
     "TRAVEL [destination] - Travel within the current star system\n" "  Without parameters: Lists all available destinations.\n" "  [destination]: The location to travel to, using the numbering system:\n" "    0       - Travel to the central star\n" "    1-8     - Travel to a planet (number depends on system)\n" "    1.1-8.5 - Travel to a station (format: planet.station)\n" "    N       - Travel to the Nav Beacon\n" "  Example: travel 2    - Travel to the second planet\n" "  Example: travel 1.3  - Travel to the third station orbiting the first planet\n" "  Example: travel N    - Travel to the Nav Beacon\n" "  Note: Travel consumes game time based on distance.\n" "        Fuel is also consumed at a rate of 0.025 liters per AU.", "STAR SYSTEM NAVIGATION", is_not_docked},

    {"dock", "d", "Dock with the current station", 
     "DOCK - Dock with the current station\n" "  Must be at a station location before docking.\n" "  Use 'travel' to navigate to a station first.\n" "  Docking provides access to market and other station services.\n" "  No parameters required.", "STAR SYSTEM NAVIGATION", is_at_station},

    {"land", NULL, "Land on a planet surface", 
     "LAND - Land on a planet surface\n" "  Allows you to land on a planet when your ship is at a planet location.\n" "  You must be at a planet before landing.\n" "  Use 'travel' to navigate to a planet first.\n" "  Landing provides access to the planet's market and services.\n" "  No parameters required.", "STAR SYSTEM NAVIGATION", is_at_planet},

    // SHIP MANAGEMENT
    {"ship", NULL, "Display basic ship status information", 
     "SHIP - Display basic ship status information\n" "  Shows hull integrity, fuel, and cargo capacity", "SHIP MANAGEMENT", is_always_available},

    {"shipinfo", NULL, "Display detailed ship information", 
     "SHIPINFO - Display detailed ship information\n" "  Shows comprehensive information about your ship's systems,\n" "  equipment, and cargo hold contents", "SHIP MANAGEMENT", is_always_available},

    {"fuelinfo", NULL, "Display detailed fuel information for your ship", 
     "FUELINFO - Display detailed fuel information for your ship\n" "  Shows current fuel level, maximum capacity, consumption rate,\n" "  estimated range, and refill cost based on your ship's specifications\n" "  This command has no parameters", "SHIP MANAGEMENT", is_always_available},

    {"repair", NULL, "Repair your ship's hull damage", 
     "REPAIR - Repair your ship's hull damage\n" "  This command will repair your ship to 100% hull integrity\n" "  Cost is 10 credits per hull point repaired\n" "  Note: You must be docked at a station to repair your ship", "SHIP MANAGEMENT", is_docked},

    {"shipyard", NULL, "View ships available for purchase", 
     "SHIPYARD - View ships available for purchase\n" "  Shows a list of ships available at the current station.\n" "  Displays hull strength, cargo capacity, and price.\n" "  Includes your current ship's trade-in value.\n" "  You must be docked at a station to use this command.\n" "  No parameters required.", "SHIP MANAGEMENT", is_docked},

    {"compareship", NULL, "Compare your ship with another ship type", 
     "COMPARESHIP <shipname> - Compare your ship with another ship type\n" "  Displays a side-by-side comparison of ship specifications.\n" "  Shows differences in hull, shields, cargo, etc.\n" "  Usage: compareship <shipname> (e.g., 'compareship Viper')\n" "  Works anywhere, docking not required.", "SHIP MANAGEMENT", is_always_available},

    {"buyship", NULL, "Purchase a new ship", 
     "BUYSHIP <ID or shipname> [notrade] - Purchase a new ship\n" "  Buys a new ship from the current station's shipyard.\n" "  <ID> - The ship ID number shown in the shipyard list\n" "  <shipname> - The name of the ship (for backward compatibility)\n" "  By default, trades in your current ship for a credit.\n" "  Use 'notrade' flag to buy without trading in (e.g., 'buyship 1 notrade').\n" "  Equipment and cargo are transferred when possible.\n" "  You must be docked at a station to use this command.\n" "  Examples: 'buyship 1' or 'buyship \"Cobra Mk III\"'", "SHIP MANAGEMENT", is_docked},

    {"upgrade", NULL, "View and purchase ship upgrades", 
     "UPGRADE [ID] [quantity] - View and purchase ship upgrades (hull, shields, etc.)\n" "  Without parameters: Lists all available upgrades\n" "  [ID]: The upgrade ID to purchase\n" "  [quantity]: Number of upgrades to purchase (default: 1)", "SHIP MANAGEMENT", is_docked},

    // EQUIPMENT AND INVENTORY
    {"equip", NULL, "Purchase and install ship equipment", 
     "EQUIP [equipment] - Purchase and install ship equipment\n" "  Without parameters: Lists all available equipment\n" "  [equipment]: The specific equipment item to purchase\n" "  Available equipment types:\n" "    ecm      - Electronic Counter Measures (600 CR)\n" "    fuelscoop - Fuel Scoop for collecting fuel from stars (525 CR)\n" "    dockcomp - Docking Computer for automated docking (1500 CR)\n" "    escape   - Escape Pod for emergency escape (1000 CR)\n" "    cargo    - Cargo Bay Extension for +4 tons capacity (400 CR)\n" "    pulse    - Pulse Laser for basic combat (400 CR)\n" "    beam     - Beam Laser for improved combat (1000 CR)\n" "    military - Military Laser for maximum firepower (2500 CR)\n" "    mining   - Mining Laser for resource extraction (800 CR)\n" "    scanner  - Scanner Upgrade for improved detection (700 CR)\n" "    missile  - Homing Missile for one-shot attacks (300 CR)\n" "  Example: equip beam\n" "  Note: You must be docked at a station to purchase equipment\n" "        Equipment availability depends on the system's tech level", "EQUIPMENT AND INVENTORY", is_docked},

    {"inv", NULL, "Display equipment inventory", 
     "INV - Display equipment inventory\n" "  Shows all equipment items stored in your ship's inventory.\n" "  Each item is shown with its inventory slot index for use with the 'use' command.", "EQUIPMENT AND INVENTORY", is_always_available},

    {"store", NULL, "Remove equipment and store in inventory", 
     "STORE <slot_number> - Remove equipment and store in inventory\n" "  <slot_number> - The equipment slot to remove equipment from\n" "  Example: store 0\n" "  Note: Use 'shipinfo' to see your equipment slots and what's installed in them.", "EQUIPMENT AND INVENTORY", is_always_available},

    {"use", NULL, "Equip item from inventory", 
     "USE <inventory_index> <slot_number> - Equip item from inventory\n" "  <inventory_index> - The inventory slot containing the equipment to use\n" "  <slot_number> - The equipment slot to install the equipment into\n" "  Example: use 2 1\n" "  Note: Equipment can only be installed in compatible slots.\n" "        Use 'inv' to see your inventory and 'shipinfo' to see slots.", "EQUIPMENT AND INVENTORY", is_always_available},

    // CARGO AND MONEY
    {"hold", "h", "Set cargo hold capacity", 
     "HOLD <amount> - Set cargo hold capacity\n" "  <amount> - Total cargo hold space in tonnes\n" "  Example: hold 20\n" "  Note: Cannot reduce hold space below current cargo volume.", "CARGO AND MONEY", is_always_available},

    {"jettison", "j", "Discard cargo into space", 
     "JETTISON <good> <amount> or JETTISON ALL - Discard cargo into space\n" "  <good>   - Type of trade good to jettison (e.g., Food, Computers)\n" "  <amount> - Quantity to jettison (default: 1)\n" "  ALL      - Special flag to jettison all cargo at once\n" "  Examples: jettison Food 5\n" "            jettison all\n" "  Note: Jettisoned cargo is lost permanently with no payment received.\n" "        Useful in emergencies or when carrying illegal goods and avoiding authorities.", "CARGO AND MONEY", is_not_docked},

    // GAME MANAGEMENT
    {"save", NULL, "Save the current game state", 
     "SAVE [description] - Save the current game state\n" "  [description] - Optional description of the save (e.g., 'At Lave')\n" "  Example: save Trading at Lave\n" "  Note: Save files are timestamped and stored in the 'saves' directory.", "GAME MANAGEMENT", is_always_available},

    {"load", NULL, "List and load saved games", 
     "LOAD - List and load saved games\n" "  Shows a list of available save files, sorted by most recent first.\n" "  Enter the number of the save file to load when prompted.\n" "  Note: Loading a save will discard your current game state.", "GAME MANAGEMENT", is_always_available},

    {"reset", NULL, "Restart the game with an optional random seed", 
     "RESET [seed] - Restart the game with an optional random seed\n" "  Without parameters: Reinitializes the game with default seed 54321\n" "  [seed]: A positive integer to use as the random seed\n" "  Example: reset, reset 12345\n" "  Note: Resetting will discard your current game state and begin a new game.", "GAME MANAGEMENT", is_always_available},

    {"quit", "q", "Exit the game", 
     "QUIT - Exit the game\n" "  Exits the game without saving. Use 'save' first to preserve your progress.", "GAME MANAGEMENT", is_always_available},

    // DEBUG COMMANDS
    {"cash", "c", "Adjust cash balance", 
     "CASH <+/-amount> - Adjust cash balance\n" "  <+/-amount> - Amount to add or subtract from cash balance\n" "  Example: cash +100.0  - Add 100 credits\n" "  Example: cash -50.5   - Subtract 50.5 credits\n" "  Note: This is a debug command for testing purposes.", "DEBUG COMMANDS", is_always_available},

    {"rand", NULL, "Toggle random number generator", 
     "RAND - Toggle random number generator\n" "  Switches between native and portable RNG implementations.\n" "  This is a debug command for testing purposes.", "DEBUG COMMANDS", is_always_available},

    {"sneak", NULL, "Jump to another system without using fuel", 
     "SNEAK <planetname> - Jump to another system without using fuel\n" "  <planetname> - Name of the destination system\n" "  Example: sneak Lave\n" "  Note: This is a debug command for testing purposes.", "DEBUG COMMANDS", is_not_docked},

    // Terminator
    {NULL, NULL, NULL, NULL, NULL, NULL}
};



// Helper function to find command help
static const CommandHelp* find_command_help(const char *command) {
    for (int i = 0; command_help[i].command != NULL; i++) {
        if (StringCompareIgnoreCase(command, command_help[i].command) == 0) {
            return &command_help[i];
        }
        // Check aliases
        if (command_help[i].aliases != NULL) {
            char aliases_copy[MAX_LEN];
            snprintf(aliases_copy, MAX_LEN, "%s", command_help[i].aliases);
            char *saveptr = NULL; // Declare saveptr for safe_strtok
            char *alias = safe_strtok(aliases_copy, " ", &saveptr);
            while (alias != NULL) {
                if (StringCompareIgnoreCase(command, alias) == 0) {
                    return &command_help[i];
                }
                alias = safe_strtok(NULL, " ", &saveptr);
            }
        }
    }
    return NULL;
}

// Helper function to print formatted help text
static void print_help_text(const char *text) {
    // Special handling for fuel command to show dynamic fuel cost
    char buffer[1024];
    if (strstr(text, "FUEL <amount>") != NULL) {
        snprintf(buffer, sizeof(buffer), text, (float)GetFuelCost() / 10.0f);
        printf("%s", buffer);
    } else {
        printf("%s", text);
    }
}

static inline bool do_tweak_random_native(char *commandArguments) {
  (void)commandArguments; // Mark 's' as unused
  NativeRand ^= 1;
  return 1;
}

static inline bool do_local_systems_display(char *commandArguments) {
  uint16_t d =
      (uint16_t)atoi(commandArguments); // commandArguments might be unused if
                                        // not parsed for a distance limit

  printf("Galaxy number %i", GalaxyNum);
  for (PlanetNum syscount = 0; syscount < GAL_SIZE; ++syscount) {
    d = distance(Galaxy[syscount], Galaxy[CurrentPlanet]);

    if (d <= MaxFuel) {
      if (d <= Fuel)
        printf("\n * ");
      else
        printf("\n - ");

      print_system_info(Galaxy[syscount], 1);
      printf(" (%.1f LY)", (float)d / 10.0f);
    }
  }
  printf("\n");
  return 1;
}

static inline bool do_jump(char *commandArguments) {
  uint16_t d;
  PlanetNum dest = find_matching_system_name(commandArguments);

  if (dest == CurrentPlanet) {
    printf("\nBad jump");
    return 0;
  }
  d = distance(Galaxy[dest], Galaxy[CurrentPlanet]);

  // Get the fuel cost per distance unit based on ship type
  int fuelCostPerUnit = GetFuelCost();

  // Calculate the fuel needed for this jump based on ship's consumption rate
  uint16_t fuelNeeded =
      (uint16_t)((float)d * ((float)fuelCostPerUnit /
                             2.0)); // Scale based on ship efficiency
  if (fuelNeeded > Fuel) {
    printf("\nJump too far - requires %d fuel units, you have %d", fuelNeeded,
           Fuel);
    return 0;
  }

  // Use the new ConsumeFuel function to update both global and ship fuel
  if (PlayerShipPtr != NULL) {
    if (!ConsumeFuel((double)fuelNeeded, 0)) {
      printf("\nJump failed: Insufficient fuel");
      return 0;
    }

    // Small chance of minor hull damage during jump
    if (rand() % 100 < 5) // 5% chance
    {
      int damageTaken = rand() % 5 + 1; // 1-5 points of damage
      PlayerShipPtr->attributes.hullStrength =
          (PlayerShipPtr->attributes.hullStrength > damageTaken)
              ? PlayerShipPtr->attributes.hullStrength - damageTaken
              : 1;

      printf("\nHyperspace stress caused minor hull damage (-%d)", damageTaken);
    }
  } // Update global Fuel based on ship's fuel if PlayerShipPtr is available
  if (PlayerShipPtr != NULL) {
    // Sync the ship's fuel with the global value (exact match)
    PlayerShipPtr->attributes.fuelLiters = Fuel * 10.0f;
  }

  execute_jump_to_planet(dest);
  print_system_info(Galaxy[CurrentPlanet], 0);
  return 1;
}

static inline bool do_sneak(char *commandArguments) {
  uint16_t fuelkeep = Fuel;
  bool b;
  Fuel = 666; // Arbitrary large fuel value for sneak
  b = do_jump(commandArguments);
  Fuel = fuelkeep;
  return b;
}

static inline bool do_galactic_hyperspace(char *commandArguments) {
  (void)(commandArguments); /* Discard s */

  // This feature has been removed from the game
  printf("\nThe Galactic Hyperspace technology has been deemed unsafe and is "
         "no longer available.");
  printf("\nPlease use standard Hyperspace jumps (jump command) for "
         "interstellar travel.");

  return 0;
}

static inline bool do_planet_info_display(char *commandArguments) {
  PlanetNum dest = find_matching_system_name(commandArguments);
  if (dest < GAL_SIZE) { // Check if a valid planet was found
    print_system_info(Galaxy[dest], 0);
  } else {
    printf("\nPlanet not found: %s", commandArguments);
    return 0;
  }
  return 1;
}

static inline bool do_hold(char *commandArguments) {
  uint16_t a = (uint16_t)atoi(commandArguments);
  uint16_t t = 0;

  for (uint16_t i = 0; i <= LAST_TRADE; ++i) {
    if ((Commodities[i].units) == TONNES_UNIT)
      t += ShipHold[i];
  }

  if (a < t) // Can't set hold space to less than current cargo
  {
    printf("\nHold too full to reduce size to %u. Current cargo: %u tonnes.", a,
           t);
    return 0;
  }

  HoldSpace = a - t;
  printf("\nHold space set to %u. Available: %u tonnes.", a, HoldSpace);
  return 1;
}

static inline bool do_sell(char *commandArguments) {
  uint16_t i;
  uint16_t t;
  char s2[MAX_LEN];
  split_string_at_first_space(commandArguments, s2);
  uint16_t a = (uint16_t)atoi(commandArguments);

  if (a == 0)
    a = 1;

  i = match_string_in_array(s2, tradnames, LAST_TRADE + 1);

  if (i == 0) {
    printf("\nUnknown trade good: '%s'", s2);
    return 0;
  }

  i -= 1; // Adjust index for 0-based array

  t = execute_sell_order(i, a);

  if (t == 0) {
    printf("Cannot sell any %s", tradnames[i]);
  } else {
    printf("\nSelling %i%s of %s", t, UnitNames[Commodities[i].units],
           tradnames[i]);

    // Synchronize the cargo systems after selling
    if (PlayerShipPtr != NULL) {
      SynchronizeCargoSystems(PlayerShipPtr);
    }
  }
  return 1;
}

static inline bool do_buy(char *commandArguments) {
  uint16_t i;
  uint16_t t;
  char s2[MAX_LEN];
  split_string_at_first_space(commandArguments, s2);
  uint16_t a = (uint16_t)atoi(commandArguments);

  if (a == 0)
    a = 1;

  i = match_string_in_array(s2, tradnames, LAST_TRADE + 1);

  if (i == 0) {
    printf("\nUnknown trade good: '%s'", s2);
    return 0;
  }
  i -= 1; // Adjust index

  t = execute_buy_order(i, a);
  if (t == 0) {
    printf("Cannot buy any %s", tradnames[i]);
  } else {
    printf("\nBuying %i%s of %s", t, UnitNames[Commodities[i].units],
           tradnames[i]);
    // Synchronize the cargo systems after buying
    if (PlayerShipPtr != NULL) {
      SynchronizeCargoSystems(PlayerShipPtr);
    }
  }
  return 1;
}

static inline bool do_fuel(char *commandArguments) {
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: fuel <amount>");
    return 0;
  }
  uint16_t f =
      calculate_fuel_purchase((uint16_t)floor(10 * atof(commandArguments)));
  if (f == 0) {
    printf("\nCan't buy any fuel");
  } else {
    // Deduct the cost from cash
    Cash -= f * GetFuelCost();

    // Add the fuel to the current fuel level, making sure not to exceed max
    // fuel for the ship
    int currentMaxFuel = GetMaxFuel();
    Fuel = (Fuel + f > currentMaxFuel) ? currentMaxFuel : Fuel + f;

    // Also update the ship's fuel levels
    if (PlayerShipPtr != NULL) {
      // Convert game units to liters (1 fuel unit = 0.1 LY = 10 liters)
      float fuelLiters = f * 10.0f;
      float maxFuelLiters = PlayerShipPtr->shipType->maxFuelLY * 100.0f;

      PlayerShipPtr->attributes.fuelLiters =
          (PlayerShipPtr->attributes.fuelLiters + fuelLiters > maxFuelLiters)
              ? maxFuelLiters
              : PlayerShipPtr->attributes.fuelLiters + fuelLiters;
    }

    printf("\nBuying %.1fLY fuel", (float)f / 10.0f);
  }
  return 1;
}

static inline bool do_cash(char *commandArguments) {
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: cash <amount>");
    return 0;
  }
  int a = (int)(10 * atof(commandArguments)); // Amount is in tenths of credits
  Cash += (long)a;

  if (a != 0) {
    printf("\nCash adjusted by %.1f. Current cash: %.1f CR.", (float)a / 10.0f,
           (float)Cash / 10.0f);
    return 1;
  }

  printf("Number not understood for cash command.");
  return 0;
}

static inline bool do_market_display(char *commandArguments) {
  (void)commandArguments;

  // Display basic market information
  display_market_info(LocalMarket);

  // Display current location economic info if we're at a station
  if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
      PlayerNavState.currentLocation.station != NULL &&
      CurrentStarSystem != NULL && CurrentStarSystem->planSys != NULL) {

    Station *station = PlayerNavState.currentLocation.station;

    // Find parent planet for context
    Planet *parentPlanet = NULL;
    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !parentPlanet;
         i++) {
      Planet *planet = &CurrentStarSystem->planets[i];
      if (!planet)
        continue;

      for (uint8_t j = 0; j < planet->numStations; j++) {
        if (planet->stations[j] == station) {
          parentPlanet = planet;
          break;
        }
      }
    }

    // Display economy information
    printf("\n\n=== STATION ECONOMY ===");
    printf("\nSystem Economy: %s",
           EconNames[CurrentStarSystem->planSys->economy]);

    // Display specialization
    const char *specNames[] = {"Balanced", "Industrial", "Agricultural",
                               "Mining"};
    if (station->specialization < 4) {
      printf("\nStation Specialization: %s",
             specNames[station->specialization]);
    }

    // Display market update time
    uint64_t timeSinceUpdate =
        game_time_get_seconds() - station->lastMarketUpdate;
    printf("\nLast Market Update: %llu seconds ago",
           (unsigned long long)timeSinceUpdate);

    // Add hint about best trades based on specialization
    printf("\n\nTrade Opportunities:");
    switch (station->specialization) {
    case 1: // Industrial
      printf("\n- Sells machinery and computers at good prices");
      printf("\n- Looking to buy food and textiles");
      break;
    case 2: // Agricultural
      printf("\n- Sells food and textiles at good prices");
      printf("\n- Looking to buy machinery and alloys");
      break;
    case 3: // Mining
      printf("\n- Sells radioactives and alloys at good prices");
      printf("\n- Looking to buy luxuries and computers");
      break;
    default: // Balanced
      printf("\n- No special trade opportunities");
      break;
    }
  }

  printf("\n\nFuel :%.1fLY", (float)Fuel / 10.0f);
  printf("      Holdspace :%ut", HoldSpace); // Cargo capacity in Tonnes
  printf("\nCurrent Cash: %.1f CR\n", (float)Cash / 10.0f);
  return 1;
}

static inline bool do_quit(char *commandArguments) {
  (void)(commandArguments);
  printf("\nExiting Text Elite. Goodbye!\n");
  exit(ExitStatus);
  // This line will not be reached if ExitStatus leads to a successful exit.
  // It's here to satisfy the function signature if exit() somehow didn't
  // terminate.
  return 1;
}

/**
 * Resets the game state to a new game, either with a custom seed or the default
 * seed. Usage: reset [seed] If no seed is provided, uses the default seed
 * "54321"
 *
 * @param commandArguments Optional seed to use for the new game
 * @return 1 if the reset was successful
 */
static inline bool do_reset(char *commandArguments) {
  unsigned int seed = 54321; // Default seed

  // If a seed is provided, use it
  if (commandArguments != NULL && commandArguments[0] != '\0') {
    unsigned int providedSeed = (unsigned int)atoi(commandArguments);
    if (providedSeed > 0) {
      seed = providedSeed;
      printf("\nResetting game with custom seed: %u", seed);
    } else {
      printf("\nInvalid seed provided. Using default seed: %u", seed);
    }
  } else {
    printf("\nResetting game with default seed: %u", seed);
  }

  // Initialize a new game with the seed
  my_srand(seed);
  initialize_player_state();
  game_time_initialize();

  printf("\nGame reset complete. You are now at planet %s in Galaxy %d.",
         Galaxy[CurrentPlanet].name, GalaxyNum);

  return 1;
}

static inline bool do_help(char *commandArguments) {
    commandArguments = strip_leading_trailing_spaces(commandArguments);

    if (commandArguments == NULL || strlen(commandArguments) == 0) {
        // Display general help
        printf("\n=== TXTELITE COMMAND REFERENCE ===\n");

        const char *current_category = NULL;
        for (int i = 0; command_help[i].command != NULL; i++) {
            if (current_category == NULL || strcmp(current_category, command_help[i].category) != 0) {
                current_category = command_help[i].category;
                printf("\n--- %s ---\n", current_category);
            }

            bool available = command_help[i].is_available();
            printf("  %s%-12s%s - %s %s\n",
                   available ? KGRN : KRED,
                   command_help[i].command,
                   KNRM,
                   command_help[i].short_description,
                   available ? "" : "(unavailable)");
        }
        printf("\nFor detailed help on any command, type 'help <command>'.\n");
    } else {
        // Display specific help for a command
        const CommandHelp *help = find_command_help(commandArguments);
        if (help) {
            print_help_text(help->long_description);
            if (!help->is_available()) {
                printf("\n%s(Currently unavailable)%s", KRED, KNRM);
            }
            printf("\n");
        } else {
            printf("\nUnknown command: %s", commandArguments);
            printf("\nUse 'help' without parameters to see all available commands.\n");
        }
    }
    return 1;
}

static inline bool do_save(char *commandArguments) {
  // Generate filename with current date and time
  char filename[64];
  time_t now = time(NULL);
  struct tm timeinfo;
  if (safe_localtime(&now, &timeinfo) == 0) {
    strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.sav", &timeinfo);
  } else {
    snprintf(filename, sizeof(filename), "save_%ld.sav", (long)now);
  }

  // Use the provided description if available
  char *description = NULL;

  // If command arguments were provided, use them as description
  if (commandArguments && commandArguments[0] != '\0') {
    description = commandArguments;
  }

  // Save the game
  bool success = save_game(filename, description);

  return success;
}

static inline bool do_load(char *commandArguments) {
  (void)commandArguments; // Unused parameter

  printf("\nAvailable save files:\n");

  // Structure to store save file information
  typedef struct {
    char filename[MAX_PATH];
    time_t timestamp;
  } SaveFileInfo;

  // Use cross-platform directory iterator
  DirectoryIterator iter;
  char searchPattern[MAX_PATH];
  platform_make_pattern(searchPattern, sizeof(searchPattern), "saves", "*.sav");

  if (!platform_find_first_file(&iter, searchPattern)) {
    printf("No save files found in the 'saves' directory.\n");
    return 0;
  }

  // Count the number of save files
  int fileCount = 0;
  SaveFileInfo saveFiles[100]; // Array to store up to 100 save files

  do {
    const char *filename = platform_get_filename(&iter);
    if (filename) {
      // Store filename
      snprintf(saveFiles[fileCount].filename, MAX_PATH, "%s", filename);

      // Get file timestamp using cross-platform function
      saveFiles[fileCount].timestamp = platform_get_file_time(&iter);

      fileCount++;
      if (fileCount >= 100)
        break; // Limit to 100 files
    }
  } while (platform_find_next_file(&iter));

  platform_find_close(&iter);

  if (fileCount == 0) {
    printf("No save files found.\n");
    return 0;
  }

  // Sort save files by timestamp (most recent first)
  for (int i = 0; i < fileCount - 1; i++) {
    for (int j = 0; j < fileCount - i - 1; j++) {
      if (saveFiles[j].timestamp < saveFiles[j + 1].timestamp) {
        // Swap
        SaveFileInfo temp = saveFiles[j];
        saveFiles[j] = saveFiles[j + 1];
        saveFiles[j + 1] = temp;
      }
    }
  }

  // Display the sorted save files
  for (int i = 0; i < fileCount;
       i++) { // Read save header to get the description
    SaveHeader header;
    char fullPath[MAX_PATH];
    platform_make_path(fullPath, sizeof(fullPath), "saves",
                       saveFiles[i].filename);
    FILE *file = safe_fopen(fullPath, "rb");
    bool headerValid = 0;

    if (file) {
      if (fread(&header, sizeof(header), 1, file) == 1) {
        headerValid = (strncmp(header.signature, SAVE_SIGNATURE,
                               strlen(SAVE_SIGNATURE)) == 0);
      }
      fclose(file);
    } // Format date and time
    struct tm timeinfo;
    int localtime_result = safe_localtime(&saveFiles[i].timestamp, &timeinfo);
    char timeStr[32];
    if (localtime_result == 0) {
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
      snprintf(timeStr, sizeof(timeStr), "Invalid Date");
    }

    printf("%2d. %s - %s", i + 1, saveFiles[i].filename, timeStr);
    if (headerValid) {
      printf(" - %s", header.description);
    }
    printf("\n");
  }

  // Prompt user to select a save file
  if (fileCount > 0) {
    printf("\nEnter the number of the save file to load (or 0 to cancel): ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
      int selection = atoi(input);
      if (selection > 0 && selection <= fileCount) {
        char fullPath[MAX_PATH];
        platform_make_path(fullPath, sizeof(fullPath), "saves",
                           saveFiles[selection - 1].filename);
        return load_game(fullPath);
      }
    }
  }

  return 0;
}

// =============================
// Star System Commands
// =============================

// Displays detailed information about the current star system, including scan
// data
static inline bool do_system_info(char *commandArguments) {
  (void)(commandArguments); // Unused parameter
  // Validate star system data
  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return 0;
  }

  // Validate pointer to PlanSys data
  if (!CurrentStarSystem->planSys) {
    printf("\nError: Planet system data not available.");
    return 0;
  } // Get current location information
  char locBuffer[MAX_LEN];
  get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

  // System header with basic information
  printf("\n==== SYSTEM SCAN: %s ====", CurrentStarSystem->planSys->name);
  printf("\nCurrent location: %s (%.2f AU from star)", locBuffer,
         PlayerNavState.distanceFromStar);

  // Economic and political information
  printf("\nEconomy: %s", EconNames[CurrentStarSystem->planSys->economy]);
  printf("\nGovernment: %s", GovNames[CurrentStarSystem->planSys->govType]);
  printf("\nTech Level: %d", CurrentStarSystem->planSys->techLev + 1);
  printf("\nPopulation: %u Billion",
         (CurrentStarSystem->planSys->population) >>
             3); // Star information with spectral classification
  const char *spectralClasses[] = {"O", "B", "A", "F", "G", "K", "M"};
  printf("\n\nStar: %s", CurrentStarSystem->centralStar.name);
  if (CurrentStarSystem->centralStar.spectralClass < 7) {
    printf("\n  Class: %s (%.1f solar masses, %.1f luminosity, %.0f K)",
           spectralClasses[CurrentStarSystem->centralStar.spectralClass],
           CurrentStarSystem->centralStar.mass,
           CurrentStarSystem->centralStar.luminosity,
           CurrentStarSystem->centralStar.temperature);
    printf("\n  Age: %.1f billion years", CurrentStarSystem->centralStar.age);
    printf("\n  Habitable Zone: %.2f - %.2f AU",
           CurrentStarSystem->centralStar.habitableZoneInner,
           CurrentStarSystem->centralStar.habitableZoneOuter);
  }

  // Planets information
  printf("\n\nPlanets: %d", CurrentStarSystem->numPlanets);
  if (CurrentStarSystem->numPlanets > 0) {
    // Planet type information for display
    const char *planetTypes[] = {"Rocky/Airless", "Terrestrial", "Gas Giant",
                                 "Ice Giant"};

    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
      Planet *planet = &CurrentStarSystem->planets[i];
      if (!planet) {
        printf("\\n  %d. [Error: Invalid planet data]", i + 1);
        continue;
      } // Planet basic info
      double distToPlanet =
          fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);      uint32_t timeToPlanet = calculate_travel_time(
          PlayerNavState.distanceFromStar, planet->orbitalDistance);
      double fuelToPlanet = calculate_travel_fuel_requirement(distToPlanet);
      printf("\n  %d. %s (%.2f AU from star, %.2f AU away, %u min travel, %.3f fuel L required)",
             i + 1, planet->name, planet->orbitalDistance, distToPlanet,
             timeToPlanet / 60, fuelToPlanet);// Planet type and physical characteristics
      if (planet->type < 4) {
        printf("\n     Type: %s", planetTypes[planet->type]);
      } else {
        printf("\n     Type: Unknown");
      }
      printf("\n     Radius: %.0f km", planet->radius);
      printf("\n     Surface Temperature: %.0f K (%.0f C)",
             planet->surfaceTemperature, planet->surfaceTemperature - 273.15);

      // Enhanced habitability analysis
      double habitabilityScore =
          calculate_habitability_score(planet, &CurrentStarSystem->centralStar);
      const char *habitabilityRating =
          get_habitability_rating(habitabilityScore);
      const char *tempCategory =
          get_temperature_category(planet->surfaceTemperature);
      bool hasAtmosphere = check_planetary_atmosphere_potential(
          planet, &CurrentStarSystem->centralStar);
      bool tidallyLocked =
          check_tidal_locking(planet, &CurrentStarSystem->centralStar);
      double radiationLevel =
          calculate_radiation_exposure(planet, &CurrentStarSystem->centralStar);

      printf("\n     Habitability: %.1f/100 (%s)", habitabilityScore,
             habitabilityRating);
      printf("\n     Temperature: %s", tempCategory);
      printf("\n     Atmosphere: %s", hasAtmosphere ? "Potential" : "Unlikely");
      printf("\n     Rotation: %s",
             tidallyLocked ? "Tidally Locked" : "Normal");
      printf("\n     Radiation: %.1fx Earth levels", radiationLevel);

      if (planet->isInHabitableZone) {
        printf("\n     Status: In Habitable Zone *");
      } else if (planet->surfaceTemperature > 273.15 &&
                 planet->surfaceTemperature < 373.15) {
        printf("\n     Status: Potentially habitable temperature");
      } else if (planet->surfaceTemperature < 200.0) {
        printf("\n     Status: Frozen world");
      } else if (planet->surfaceTemperature > 500.0) {
        printf("\n     Status: Scorched world");
      } // Station information for this planet
      if (planet->numStations > 0) {
        printf("\n     Stations: %d", planet->numStations);

        bool hasValidStations = 0;
        for (uint8_t j = 0; j < planet->numStations; j++) {
          Station *station = planet->stations[j];
          if (!station)
            continue; // Skip NULL stations

          hasValidStations = 1; // Station type information
          const char *stationTypes[] = {"Orbital", "Coriolis", "Ocellus"};
          double stationDistAbsolute =
              planet->orbitalDistance + station->orbitalDistance;
          double distToStation =
              fabs(PlayerNavState.distanceFromStar - stationDistAbsolute);          uint32_t timeToStation = calculate_travel_time(
              PlayerNavState.distanceFromStar, stationDistAbsolute);
          double fuelToStation =
              calculate_travel_fuel_requirement(distToStation);

          printf("\n     %d.%d. %s (%.3f AU from planet, %.2f AU away, %u min "
                 "travel, %.3f fuel L required)",
                 i + 1, j + 1, station->name, station->orbitalDistance,
                 distToStation, timeToStation / 60, fuelToStation);

          // Display station type if valid
          if (station->type < 3) {
            printf("\n          Type: %s", stationTypes[station->type]);
          }

          // List available services
          printf("\n          Services: ");
          if (station->hasMarket)
            printf("Market ");
          if (station->hasShipyard)
            printf("Shipyard ");
          if (station->hasMissions)
            printf("Missions ");
          if (station->hasDockingComputer)
            printf("DockingComputer ");
          if (!station->hasMarket && !station->hasShipyard &&
              !station->hasMissions && !station->hasDockingComputer) {
            printf("None");
          }
        }

        if (!hasValidStations) {
          printf("\n     [No valid stations data]");
        }
      } else {
        printf("\n     Stations: None");
      }
    }
  } else {
    printf("\n  (None)");
  } // Nav Beacon information
  double distToNavBeacon = fabs(PlayerNavState.distanceFromStar -
                                CurrentStarSystem->navBeaconDistance);  uint32_t timeToNavBeacon = calculate_travel_time(
      PlayerNavState.distanceFromStar, CurrentStarSystem->navBeaconDistance);
  double fuelToNavBeacon = calculate_travel_fuel_requirement(distToNavBeacon);
  printf("\n\nNav Beacon: %.2f AU from star (%.2f AU away, %u min travel, %.3f fuel L required)",
         CurrentStarSystem->navBeaconDistance, distToNavBeacon,
         timeToNavBeacon / 60, fuelToNavBeacon);
  printf("\n  Travel code: N");

  // Current player location
  printf("\n\nCurrent location: %s (%.2f AU from star)", locBuffer,
         PlayerNavState.distanceFromStar);
  // Star distance and travel time (from current location)
  double distToStar = PlayerNavState.distanceFromStar;
  uint32_t timeToStar =
      calculate_travel_time(PlayerNavState.distanceFromStar, 0.0);  double fuelToStar = calculate_travel_fuel_requirement(distToStar);
  printf("\nDistance to Star (%s): %.2f AU, %u min travel, %.3f fuel L required",
         CurrentStarSystem->centralStar.name, distToStar, timeToStar / 60,
         fuelToStar);
  printf("\n  Travel code: 0");

  // Add travel hint
  printf("\n\n(Use 'travel <code>' to navigate to any location, e.g., 'travel "
         "2.1' or 'travel N')");

  // System time
  char timeBuffer[MAX_LEN * 2];
  game_time_get_formatted(timeBuffer, sizeof(timeBuffer));
  printf("\n\nSystem Time: %s", timeBuffer);

  // Small time cost for performing a system scan (1 minute)
  game_time_advance(60);
  printf("\n\nSystem scan complete. Elapsed time: 1 minute.");

  return 1;
}

// Lists available destinations and allows travel within the system
static inline bool do_travel(char *commandArguments) {
  // Check if star system data is properly initialized
  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return 0;
  }

  // Get current location information for display
  char locBuffer[MAX_LEN];
  get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

  // If no arguments provided, just list destinations
  if (commandArguments == NULL || strlen(commandArguments) == 0 ||
      strspn(commandArguments, " \t\n\r") == strlen(commandArguments)) {

    printf("\nCurrent location: %s (%.2f AU from star)", locBuffer,
           PlayerNavState.distanceFromStar);
    printf("\n\nAvailable destinations:");

    // Star
    printf("\n  0. %s (0.00 AU)", CurrentStarSystem->centralStar.name);

    // Planets and stations
    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
      Planet *planet = &CurrentStarSystem->planets[i];
      if (!planet) {
        printf("\n  %d. [Error: Invalid planet data]", i + 1);
        continue;
      }
      printf("\n  %d. %s (%.2f AU)", i + 1, planet->name,
             planet->orbitalDistance);

      for (uint8_t j = 0; j < planet->numStations; j++) {
        Station *station = planet->stations[j];
        if (!station) {
          continue; // Skip invalid stations
        }
        printf("\n     %d.%d. %s (%.2f AU)", i + 1, j + 1, station->name,
               planet->orbitalDistance + station->orbitalDistance);
      }
    }

    // Nav Beacon
    printf("\n  N. Nav Beacon (%.2f AU)", CurrentStarSystem->navBeaconDistance);

    printf("\n\nUse 'travel <destination number>' to travel (e.g., 'travel 1' "
           "or 'travel 1.2' or 'travel N')");
    return 1;
  }
  // Parse destination string, trimming whitespace
  char destStr[MAX_LEN];
  snprintf(destStr, sizeof(destStr), "%s", commandArguments);

  // Trim leading and trailing whitespace
  char *start = destStr;
  char *end = destStr + strlen(destStr) - 1;

  while (*start && isspace((unsigned char)*start))
    start++;
  while (end > start && isspace((unsigned char)*end))
    *end-- = '\0';

  if (start != destStr) {
    memmove(destStr, start, strlen(start) + 1);
  }

  // If destination string is empty after trimming
  if (strlen(destStr) == 0) {
    printf("\nNo destination specified. Use 'travel' to see available "
           "destinations.");
    return 0;
  }

  // Check for Nav Beacon special case
  if (destStr[0] == 'N' ||
      destStr[0] == 'n') { // Check if already at Nav Beacon
    if (PlayerNavState.currentLocationType == CELESTIAL_NAV_BEACON) {
      printf("\nAlready at Nav Beacon.");
      return 1;
    }
    // Calculate fuel requirement
    double distanceDelta = fabs(PlayerNavState.distanceFromStar -
                                CurrentStarSystem->navBeaconDistance);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to Nav Beacon... (Fuel required: %.3f liters)",
           fuelRequired);

    // Pass a dummy non-NULL pointer for consistency with the function signature
    void *dummy = &CurrentStarSystem; // Using any valid address as a dummy
    bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                      CELESTIAL_NAV_BEACON, dummy);
    if (result) {
      printf("\nArrived at Nav Beacon (%.2f AU from star)",
             PlayerNavState.distanceFromStar);
      return 1;
    } else {
      printf("\nFailed to travel to Nav Beacon.");
      return 0;
    }
  }

  // Parse destination index(es) for planets and stations
  int primaryIndex = -1;
  int secondaryIndex = -1;

  // Check for format "1.2" (planet.station)
  char *dotPos = strchr(destStr, '.');
  if (dotPos) {
    *dotPos = '\0'; // Split string at the dot

    // Validate that we have valid digits
    for (char *p = destStr; *p; p++) {
      if (!isdigit((unsigned char)*p)) {
        printf("\nInvalid planet number: %s. Must be a number.", destStr);
        return 0;
      }
    }

    for (char *p = dotPos + 1; *p; p++) {
      if (!isdigit((unsigned char)*p)) {
        printf("\nInvalid station number: %s. Must be a number.", dotPos + 1);
        return 0;
      }
    }

    primaryIndex = atoi(destStr);
    secondaryIndex = atoi(dotPos + 1);

    // Validate index ranges
    if (primaryIndex <= 0) {
      printf("\nInvalid planet number: %d. Must be a positive number.",
             primaryIndex);
      return 0;
    }

    if (secondaryIndex <= 0) {
      printf("\nInvalid station number: %d. Must be a positive number.",
             secondaryIndex);
      return 0;
    }
  } else {
    // For just a planet or star, validate that we have valid digits or '0'
    if (strcmp(destStr, "0") == 0) {
      primaryIndex = 0;
    } else {
      for (char *p = destStr; *p; p++) {
        if (!isdigit((unsigned char)*p)) {
          printf("\nInvalid destination number: %s. Must be a number or 'N' "
                 "for Nav Beacon.",
                 destStr);
          return 0;
        }
      }
      primaryIndex = atoi(destStr);

      if (primaryIndex < 0) {
        printf(
            "\nInvalid destination number: %d. Must be a non-negative number.",
            primaryIndex);
        return 0;
      }
    }
  }

  // Special case for star (index 0)
  if (primaryIndex == 0) { // Check if already at star
    if (PlayerNavState.currentLocationType == CELESTIAL_STAR) {
      printf("\nAlready at %s.", CurrentStarSystem->centralStar.name);    return 1;
    }
    // Calculate fuel requirement
    double distanceDelta =
        PlayerNavState
            .distanceFromStar; // Distance to star is just current distance
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to %s... (Fuel required: %.3f liters)",
           CurrentStarSystem->centralStar.name, fuelRequired);
    bool result =
        travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_STAR,
                            &CurrentStarSystem->centralStar);
    if (result) {
      printf("\nArrived at %s (0.00 AU from star)",
             CurrentStarSystem->centralStar.name);
      return 1;
    } else {
      printf("\nFailed to travel to %s.", CurrentStarSystem->centralStar.name);
      return 0;
    }
  }

  // Adjust for 1-based indexing for planets
  primaryIndex--;

  // Check if planet index is valid
  if (primaryIndex < 0 || primaryIndex >= CurrentStarSystem->numPlanets) {
    printf("\nInvalid destination. Planet number %d does not exist in this "
           "system.",
           primaryIndex + 1);
    printf("\nThis system has %d planets. Use 'travel' to see available "
           "destinations.",
           CurrentStarSystem->numPlanets);
    return 0;
  }

  Planet *planet = &CurrentStarSystem->planets[primaryIndex];
  if (!planet) {
    printf("\nError: Invalid planet data for planet %d.", primaryIndex + 1);
    return 0;
  }

  // If no secondary index, travel to planet
  if (secondaryIndex == -1) { // Check if already at this planet
    if (PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
    PlayerNavState.currentLocation.planet == planet) {
      printf("\nAlready at %s.", planet->name);
      return 1;
    }
    // Calculate fuel requirement
    double distanceDelta =
        fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to %s... (Fuel required: %.3f liters)",
           planet->name, fuelRequired);
    bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                      CELESTIAL_PLANET, planet);
    if (result) {
      printf("\nArrived at %s (%.2f AU from star)", planet->name,
             PlayerNavState.distanceFromStar);
      return 1;
    } else {
      printf("\nFailed to travel to %s.", planet->name);
      return 0;
    }
  }

  // Adjust for 1-based indexing for stations
  secondaryIndex--;

  // Check if station index is valid
  if (secondaryIndex < 0 || secondaryIndex >= planet->numStations) {
    printf("\nInvalid station. Planet %s has %d stations (numbered 1 to %d).",
           planet->name, planet->numStations, planet->numStations);
    return 0;
  }

  Station *station = planet->stations[secondaryIndex];
  if (!station) {
    printf("\nError: Station data not available for station %d of planet %s.",
           secondaryIndex + 1, planet->name);
    return 0;
  }
  // Check if already at this station
  if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
      PlayerNavState.currentLocation.station == station) {
    printf("\nAlready at %s.", station->name);
    return 1;
  }
  // Calculate fuel requirement
  double stationDistance = planet->orbitalDistance + station->orbitalDistance;
  double distanceDelta =
      fabs(PlayerNavState.distanceFromStar - stationDistance);
  double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

  printf("\nTravelling to %s... (Fuel required: %.3f liters)",
         station->name, fuelRequired);
  bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                    CELESTIAL_STATION, station);
  if (result) {
    printf("\nArrived at %s (%.2f AU from star)", station->name,
           PlayerNavState.distanceFromStar);
    return 1;
  } else {
    printf("\nFailed to travel to %s.", station->name);
    return 0;
  }
}

// Docks with a station if at a station location
static inline bool do_dock(char *commandArguments) {
  (void)(commandArguments); // Unused parameter

  // Validate star system data
  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return 0;
  }

  // Check the player's current location type
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION) {
    // Provide a helpful message based on current location
    char locBuffer[MAX_LEN];
    get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

    printf("\nCannot dock: Not at a station. You are currently at %s.",
           locBuffer);
    printf("\nUse 'travel' to navigate to a station first.");

    // List nearby stations as a convenience
    bool stationsFound = 0;
    printf("\n\nNearby stations:");

    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
      Planet *planet = &CurrentStarSystem->planets[i];
      if (!planet)
        continue;

      for (uint8_t j = 0; j < planet->numStations; j++) {
        Station *station = planet->stations[j];
        if (!station)
          continue;

        double stationDist = planet->orbitalDistance + station->orbitalDistance;
        double distToStation =
            fabs(PlayerNavState.distanceFromStar - stationDist);

        // Show stations within 1 AU as "nearby"
        if (distToStation <= 1.0) {
          printf("\n  %s (%.2f AU away) - Use 'travel %d.%d' to reach",
                 station->name, distToStation, i + 1, j + 1);
          stationsFound = 1;
        }
      }
    }

    if (!stationsFound) {
      printf("\n  No stations within 1 AU. Use 'scan' to find all stations in "
             "the system.");
    }

    return 0;
  }

  // Validate station data
  Station *station = PlayerNavState.currentLocation.station;
  if (!station) {
    printf("\nError: Station data not available. Cannot complete docking "
           "procedure.");
    return 0;
  }

  // Find the parent planet for better location context
  Planet *parentPlanet = NULL;
  for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !parentPlanet; i++) {
    Planet *planet = &CurrentStarSystem->planets[i];
    if (!planet)
      continue;

    for (uint8_t j = 0; j < planet->numStations; j++) {
      if (planet->stations[j] == station) {
        parentPlanet = planet;
        break;
      }
    }
  } // Docking procedure and feedback
  printf("\nDocking at %s...", station->name);

  // Small time delay for docking
  game_time_advance(60); // 1 minute

  // Update the global docking status variable
  extern int PlayerLocationType;
  PlayerLocationType = 10; // 10 = docked at station

  printf("\nDocked successfully. Welcome to %s!", station->name);

  // If we have parent planet info, display it
  if (parentPlanet) {
    printf("\nLocation: Orbiting %s", parentPlanet->name);

    // Update and use this station's market if it has one
    if (station->hasMarket && CurrentStarSystem->planSys) {
      // Update the station's market to the current game time
      UpdateStationMarket(station, game_time_get_seconds(), parentPlanet,
                          CurrentStarSystem->planSys);

      // Set the global market to this station's market
      UseStationMarket(station, parentPlanet, CurrentStarSystem->planSys);

      // Show economic specialization
      const char *specNames[] = {"Balanced", "Industrial", "Agricultural",
                                 "Mining"};
      if (station->specialization < 4) {
        printf("\nEconomic specialization: %s",
               specNames[station->specialization]);
      }
    }
  }

  // Display available services
  printf("\n\nAvailable services:");
  if (station->hasMarket) {
    printf("\n- Market (use 'mkt', 'buy', 'sell' commands)");
  }
  if (station->hasShipyard) {
    printf("\n- Shipyard (equipment upgrades available)");
  }
  if (station->hasMissions) {
    printf("\n- Mission Board (missions available)");
  }
  if (station->hasDockingComputer) {
    printf("\n- Docking Computer Installation");
  }

  // If no services are available
  if (!station->hasMarket && !station->hasShipyard && !station->hasMissions &&
      !station->hasDockingComputer) {
    printf("\n- No services available at this station");
  }

  // Additional contextual information
  printf("\n\nLocal system time: ");
  char timeBuffer[MAX_LEN * 2];
  game_time_get_formatted(timeBuffer, sizeof(timeBuffer));
  printf("%s", timeBuffer);

  return 1;
}

/**
 * Lands on a planet surface, similar to docking at a station.
 * This establishes a planetside base for trading and other activities.
 */
static inline bool do_land(char *commandArguments) {
  (void)(commandArguments); // Unused parameter

  // Validate star system data

  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return 0;
  }

  // Check the player's current location type
  if (PlayerNavState.currentLocationType != CELESTIAL_PLANET ||
      !PlayerNavState.currentLocation.planet) {
    // Provide a helpful message based on current location
    char locBuffer[MAX_LEN];
    get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

    printf("\nCannot land: Not at a planet. You are currently at %s.",
           locBuffer);
    printf("\nUse 'travel' to navigate to a planet first.");

    // List nearby planets as a convenience
    bool planetsFound = 0;
    printf("\n\nNearby planets:");

    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
      Planet *planet = &CurrentStarSystem->planets[i];
      if (!planet)
        continue;

      double distToPlanet =
          fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);

      // Show planets within 1 AU as "nearby"
      if (distToPlanet <= 1.0) {
        printf("\n  %s (%.2f AU away) - Use 'travel %d' to reach", planet->name,
               distToPlanet, i + 1);
        planetsFound = 1;
      }
    }

    if (!planetsFound) {
      printf("\n  No planets within 1 AU. Use 'scan' to find all planets in "
             "the system.");
    }

    return 0;
  }
  // At this point, we're at a planet and can land
  Planet *planet = PlayerNavState.currentLocation.planet;

  // Check if the planet type allows landing
  if (planet->type == 2) { // Gas Giant
    printf("\nCannot land on %s: Gas giants have no solid surface to land on.", planet->name);
    printf("\nGas giants are composed primarily of gases and liquids with no accessible solid surface.");
    printf("\nYou can only land on rocky or terrestrial planets.");
    printf("\nUse 'travel' to navigate to a different planet, or dock at an orbital station instead.");
    
    // Show nearby stations around this gas giant as alternatives
    if (planet->numStations > 0) {
      printf("\n\nOrbital stations around %s:", planet->name);
      for (uint8_t j = 0; j < planet->numStations; j++) {
        Station *station = planet->stations[j];
        if (station) {
          double stationDist = planet->orbitalDistance + station->orbitalDistance;
          double distToStation = fabs(PlayerNavState.distanceFromStar - stationDist);
          printf("\n  %s (%.3f AU away) - Use 'travel %d.%d' to dock", 
                 station->name, distToStation, 
                 // Find planet index for this planet
                 (int)(planet - CurrentStarSystem->planets) + 1, j + 1);
        }
      }
    }
    return 0;
  }
  
  if (planet->type == 3) { // Ice Giant  
    printf("\nCannot land on %s: Ice giants have no accessible solid surface.", planet->name);
    printf("\nIce giants are composed primarily of water, methane, and ammonia ices in a thick atmosphere.");
    printf("\nAny solid surface is buried under thousands of kilometers of dense atmosphere and ice.");
    printf("\nYou can only land on rocky or terrestrial planets.");
    printf("\nUse 'travel' to navigate to a different planet, or dock at an orbital station instead.");
    
    // Show nearby stations around this ice giant as alternatives
    if (planet->numStations > 0) {
      printf("\n\nOrbital stations around %s:", planet->name);
      for (uint8_t j = 0; j < planet->numStations; j++) {
        Station *station = planet->stations[j];
        if (station) {
          double stationDist = planet->orbitalDistance + station->orbitalDistance;
          double distToStation = fabs(PlayerNavState.distanceFromStar - stationDist);
          printf("\n  %s (%.3f AU away) - Use 'travel %d.%d' to dock", 
                 station->name, distToStation,
                 // Find planet index for this planet  
                 (int)(planet - CurrentStarSystem->planets) + 1, j + 1);
        }
      }
    }
    return 0;
  }

  // Landing procedure and feedback
  printf("\nLanding on %s...", planet->name);

  // Small time delay for landing
  game_time_advance(120); // 2 minutes to land

  printf("\nLanded successfully. Welcome to %s!", planet->name);

  // Initialize or update the planet's market
  if (CurrentStarSystem->planSys) {

    // Generate or update the planet's market using the correct functions
    if (!planet->planetaryMarket.isInitialized) {
      // Set market fluctuation for this planet
      planet->marketFluctuation =
          (CurrentStarSystem->planSys->goatSoupSeed.c + planet->type) % 16;
      planet->lastMarketUpdate = game_time_get_seconds();

      // Create a temporary station to use the market generation function
      Station tempStation;
      memset(&tempStation, 0, sizeof(Station));
      tempStation.marketFluctuation = planet->marketFluctuation;

      // Set specialization based on planet type
      if (planet->type <= 1) { // Rocky or Terrestrial
        tempStation.specialization =
            2; // Agricultural focus for terrestrial planets
      } else {
        tempStation.specialization =
            3; // Mining focus for gas giants and ice planets
      }

      // Generate market and store in planet's market
      tempStation.market = GenerateStationMarket(&tempStation, planet,
                                                 CurrentStarSystem->planSys);
      planet->planetaryMarket.market = tempStation.market;
      planet->planetaryMarket.isInitialized = 1;
    } else {
      // Update existing market based on elapsed time
      uint64_t currentTime = game_time_get_seconds();

      // Only update if sufficient time has passed (at least 1 hour of game
      // time)
      const uint64_t UPDATE_INTERVAL = 3600; // 1 hour in seconds

      if (currentTime - planet->lastMarketUpdate >= UPDATE_INTERVAL) {
        // Create temporary station for market update
        Station tempStation;
        memset(&tempStation, 0, sizeof(Station));
        tempStation.marketFluctuation = planet->marketFluctuation;
        tempStation.market = planet->planetaryMarket.market;
        tempStation.lastMarketUpdate = planet->lastMarketUpdate;

        // Set specialization based on planet type
        if (planet->type <= 1) {          // Rocky or Terrestrial
          tempStation.specialization = 2; // Agricultural focus
        } else {
          tempStation.specialization = 3; // Mining focus
        }

        // Update market using station market update function
        UpdateStationMarket(&tempStation, currentTime, planet,
                            CurrentStarSystem->planSys);

        // Store updated market back in planet
        planet->planetaryMarket.market = tempStation.market;
        planet->lastMarketUpdate = currentTime;
      }
    }

    // Set the local market to the planet's market
    LocalMarket = planet->planetaryMarket.market;

    // Show information about the planet
    printf("\n\n=== PLANET INFORMATION ===");

    // Display planet type
    const char *planetTypes[] = {"Rocky/Airless", "Terrestrial", "Gas Giant",
                                 "Ice Planet"};
    if (planet->type < 4) {
      printf("\nPlanet Type: %s", planetTypes[planet->type]);
    }

    // Show economy information
    printf("\nSystem Economy: %s",
           EconNames[CurrentStarSystem->planSys->economy]);

    // Display resource specialization based on planet type
    const char *resourceTypes[] = {"Minerals", "Agriculture", "Gases",
                                   "Rare Elements"};
    printf("\nMain Resources: %s", resourceTypes[planet->type % 4]);

    // Display market update time
    uint64_t timeSinceUpdate =
        game_time_get_seconds() - planet->lastMarketUpdate;
    printf("\nLast Market Update: %llu seconds ago",
           (unsigned long long)timeSinceUpdate);

    printf("\n\nTrading post established. Use 'mkt' to view available goods.");
  }

  return 1;
}

static inline void update_all_system_markets() {
  // Check if star system data is properly initialized

  if (!CurrentStarSystem) {
    return;
  }

  // Get current game time
  uint64_t currentTime = game_time_get_seconds();

  // Update markets for all stations in the system
  for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
    Planet *planet = &CurrentStarSystem->planets[i];
    if (!planet)
      continue;

    for (uint8_t j = 0; j < planet->numStations; j++) {
      Station *station = planet->stations[j];
      if (!station)
        continue;

      // Update this station's market
      UpdateStationMarket(station, currentTime, planet,
                          CurrentStarSystem->planSys);
    }
  }
}

// Compare markets across different stations in the system or with the planet
// market
static inline bool do_compare_markets(char *commandArguments) {
  (void)commandArguments; // Mark as unused
  if (!CurrentStarSystem ||
      !CurrentStarSystem->planSys) // Added planSys check for safety
  {
    printf("\\nError: Star system data not available for market comparison.");
    return 0;
  }

  MarketType baseMarketToCompare; // Changed MarketInfo to MarketType
  char baseLocationName[MAX_LEN];
  bool isPlanetBase = 0; // Initialize isPlanetBase

  // Determine the base market for comparison
  if (PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
      PlayerNavState.currentLocation.planet) {
    Planet *currentPlanet = PlayerNavState.currentLocation.planet;
    if (!currentPlanet) {
      printf("\nError: Current planet data is invalid for comparison.");
      return 0;
    }

    snprintf(baseLocationName, MAX_LEN, "%s", currentPlanet->name);
    isPlanetBase = 1;

    // Ensure the planetary market is initialized and up-to-date.
    // UpdatePlanetaryMarket handles both initialization and updates.
    // Use local market data since we're at the planet
    baseMarketToCompare = LocalMarket;
  } else if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
             PlayerNavState.currentLocation.station) {
    Station *currentStation = PlayerNavState.currentLocation.station;
    if (!currentStation) {
      printf("\nError: Current station data is invalid for comparison.");
      return 0;
    }
    snprintf(baseLocationName, MAX_LEN, "%s", currentStation->name);
    isPlanetBase = 0;

    Planet *orbitingPlanet = NULL;
    // Find the planet this station orbits for market context
    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; ++i) {
      Planet *p = &CurrentStarSystem->planets[i];
      if (!p)
        continue;
      for (uint8_t j = 0; j < p->numStations; ++j) {
        if (p->stations[j] == currentStation) {
          orbitingPlanet = p;
          break;
        }
      }
      if (orbitingPlanet)
        break;
    }

    if (orbitingPlanet) {
      // Ensure the station market is initialized and up-to-date.
      UpdateStationMarket(currentStation, game_time_get_seconds(),
                          orbitingPlanet, CurrentStarSystem->planSys);
      // Use the local market (which should be set to this station's market)
      baseMarketToCompare = LocalMarket;
    } else {
      printf("\nError: Could not determine orbiting planet for station %s. "
             "Using potentially stale local market data.",
             currentStation->name);
      // Fallback to LocalMarket if orbiting planet not found.
      baseMarketToCompare = LocalMarket;
    }
  } else {
    printf("\nYou must be docked at a station or landed on a planet to compare "
           "markets.");
    return 0;
  }

  printf("\n=== MARKET COMPARISON ===");
  printf("\nBase location: %s", baseLocationName);

  bool foundStationsToCompare = 0;
  for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++) {
    Planet *planet = &CurrentStarSystem->planets[i];
    if (!planet)
      continue;

    for (uint8_t j = 0; j < planet->numStations; j++) {
      Station *station = planet->stations[j];
      if (!station || !station->hasMarket)
        continue; // Skip stations without markets

      // Skip comparing base station to itself if the base is a station
      if (!isPlanetBase && PlayerNavState.currentLocation.station == station) {
        continue;
      }

      foundStationsToCompare = 1;
      // Update the "other" station's market to current time to ensure fair
      // comparison
      UpdateStationMarket(station, game_time_get_seconds(), planet,
                          CurrentStarSystem->planSys);

      // Create a temporary market for comparison
      MarketType otherMarket;

      // Use the current station's market
      // This assumes UpdateStationMarket updates the station's market data
      // directly
      UseStationMarket(station, planet, CurrentStarSystem->planSys);
      otherMarket = station->market;

      printf("\n\nStation: %s (Orbiting %s)", station->name, planet->name);
      printf("\n-----------------------------------");
      printf("\n%-12s %-8s %-8s %-8s %-8s", "Commodity", "Base", "Other",
             "Diff", "QtyDiff");

      for (uint16_t k = 0; k <= LAST_TRADE; k++) {
        // Skip invalid commodities
        if (Commodities[k].basePrice == 0)
          continue;

        double basePrice =
            baseMarketToCompare.price[k]; // Changed Price to price
        int baseQty =
            baseMarketToCompare.quantity[k]; // Changed Quantity to quantity

        double otherPrice = otherMarket.price[k]; // Changed Price to price
        int otherQty = otherMarket.quantity[k]; // Changed Quantity to quantity

        printf("\n%-12s %-8.1f %-8.1f %-8.1f %-8d", tradnames[k],
               (float)basePrice / 10.0f, (float)otherPrice / 10.0f,
               (float)(otherPrice - basePrice) / 10.0f, otherQty - baseQty);
      }
    }
  }

  if (!foundStationsToCompare) {
    if (isPlanetBase) {
      printf("\n\nNo other stations in the system with markets to compare "
             "against %s.",
             baseLocationName);
    } else {
      printf("\n\nNo other stations in the system with markets to compare "
             "against your current station %s.",
             baseLocationName);
      printf("\nOr you are at the only station with a market.");
    }
  }

  if (isPlanetBase) {
    printf("\n\nNote: Comparing all stations in the system to the planet "
           "market at %s.",
           baseLocationName);
  } else {
    printf("\n\nNote: Comparing all other stations in the system to your "
           "current station %s.",
           baseLocationName);
  }

  // Restore the original local market
  if (!isPlanetBase && PlayerNavState.currentLocation.station) {
    // Find planet for current station
    Planet *currentPlanet = NULL;
    Station *currentStation = PlayerNavState.currentLocation.station;

    for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !currentPlanet;
         i++) {
      Planet *p = &CurrentStarSystem->planets[i];
      if (!p)
        continue;

      for (uint8_t j = 0; j < p->numStations; j++) {
        if (p->stations[j] == currentStation) {
          currentPlanet = p;
          break;
        }
      }
    }

    if (currentPlanet) {
      UseStationMarket(currentStation, currentPlanet,
                       CurrentStarSystem->planSys);
    }
  }

  return 1;
}

static inline bool do_ship_status(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return 0;
  }

  // Note: We don't synchronize fuel here as the ship's fuel value should be
  // more precise and is the source of truth after travel operations

  // Display basic ship information
  printf("\n=== Ship Status: %s (%s) ===", PlayerShipPtr->shipName,
         PlayerShipPtr->shipClassName);

    // Display hull
  int hullPercentage = (PlayerShipPtr->attributes.hullStrength * 100) /
                       PlayerShipPtr->shipType->baseHullStrength;
  printf("\nHull Integrity: %d%%", hullPercentage);
  // Display fuel
  double currentFuelLY = PlayerShipPtr->attributes.fuelLiters / 100.0;
  double maxFuelLY = PlayerShipPtr->shipType->maxFuelLY;
  double fuelPercent = (currentFuelLY / maxFuelLY) * 100.0;

  printf("\nFuel: %.1f/%.1f LY (%.0f%%) - Consumption: %.1f CR per 0.1 LY",
         currentFuelLY, maxFuelLY, fuelPercent,
         PlayerShipPtr->shipType->fuelConsumptionRate / 10.0);

  // Display cargo
  printf("\nCargo Capacity: %d/%d tons",
         PlayerShipPtr->attributes.currentCargoTons,
         PlayerShipPtr->attributes.cargoCapacityTons);
  // Display equipment
  printf("\n\n=== Equipment ===");
  bool hasEquipment = 0;
  // Check all equipment slots for active equipment
  for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
    if (PlayerShipPtr->equipment[i].isActive &&
        strlen(PlayerShipPtr->equipment[i].name) > 0 &&
        strcmp(PlayerShipPtr->equipment[i].name, "Empty") != 0) {

      hasEquipment = 1;
      printf("\n  - %s", PlayerShipPtr->equipment[i].name);
    }
  }

  if (!hasEquipment) {
    printf("\n  No active equipment.");
  }

  printf("\n");
  return 1;
}

static inline bool do_repair(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return 0;
  }

  // Check if repair is needed
  if (PlayerShipPtr->attributes.hullStrength >=
      PlayerShipPtr->shipType->baseHullStrength) {
    printf("\nYour ship doesn't need any repairs.");
    return 1;
  }

  // Calculate repair cost - 10 credits per unit of hull damage
  int damageAmount = PlayerShipPtr->shipType->baseHullStrength -
                     PlayerShipPtr->attributes.hullStrength;
  int repairCost = damageAmount * 10;

  // Check if player can afford repairs
  if (Cash < repairCost * 10) // Convert to internal units
  {
    printf("\nYou can't afford the repairs. Cost: %.1f credits",
           (float)repairCost);
    return 0;
  }

  // Perform the repair
  Cash -= repairCost * 10; // Convert to internal units
  PlayerShipPtr->attributes.hullStrength =
      PlayerShipPtr->shipType->baseHullStrength;

  printf("\nShip repaired for %.1f credits. Hull integrity restored to 100%%.",
         (float)repairCost);
  return 1;
}

static inline bool do_ship_details(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return 0;
  }

  // Synchronize ship fuel with global state before displaying details
  extern uint16_t Fuel;
  PlayerShipPtr->attributes.fuelLiters =
      Fuel * 10.0; // Convert game units to liters

  // Call the detailed ship status display function from elite_ship_types.h
  DisplayShipStatus(PlayerShipPtr);
  return 1;
}

/**
 * Command to purchase and install equipment on the player's ship.
 * Usage: equip <equipment_name>
 * Available equipment depends on the current system's tech level.
 */
static inline bool do_purchase_equipment(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return 0;
  }

  // Check if we are docked at a station
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION) {
    printf("\nYou must be docked at a station to purchase equipment.");
    return 0;
  }

  // Check if an equipment name was provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: equip <equipment_name>");
    printf("\n\nAvailable Equipment:");    printf("\n- ecm          - Electronic Counter Measures (600 CR)");
    printf("\n- fuelscoop    - Fuel Scoop (525 CR)");
    printf("\n- dockcomp     - Docking Computer (1500 CR)");
    printf("\n- escape       - Escape Pod (1000 CR)");
    printf("\n- cargo        - Cargo Bay Extension (400 CR)");
    printf("\n- pulse        - Pulse Laser (400 CR)");
    printf("\n- beam         - Beam Laser (1000 CR)");
    printf("\n- military     - Military Laser (2500 CR)");
    printf("\n- mining       - Mining Laser (800 CR)");
    printf("\n- scanner      - Scanner Upgrade (700 CR)");
    printf("\n- missile      - Homing Missile (300 CR)");
    printf("\n  Example: equip beam");
    printf("\n  Note: You must be docked at a station to purchase equipment");
    printf(
        "\n        Equipment availability depends on the system's tech level");
    return 1;
  }
  // Normalize input to lowercase for case-insensitive matching
  char equipName[MAX_LEN];
  snprintf(equipName, MAX_LEN, "%s", commandArguments);

  // Convert to lowercase
  for (char *p = equipName; *p; ++p) {
    *p = tolower(*p);
  }

  // External state variables    extern int32_t Cash;
  extern struct PlanSys Galaxy[];
  extern int CurrentPlanet; // Using int as defined in elite_state.h

  // Get current system tech level (0-based index)
  int techLevel = Galaxy[CurrentPlanet].techLev;
  // Prepare equipment parameters for purchase
  EquipmentTypeSpecifics equipType;
  EquipmentSlotType slotType = EQUIPMENT_SLOT_TYPE_NONE;
  const char *formalName = NULL;
  int cost = 0;
  int requiredTechLevel = 0;
  double damageOutput = 0.0;

  // Match equipment name to available options
  if (strcmp(equipName, "ecm") == 0) {    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    slotType = EQUIPMENT_SLOT_TYPE_DEFENSIVE_1;
    formalName = "ECM System";
    cost = COST_ECM;
    requiredTechLevel = 2;
  } else if (strcmp(equipName, "fuelscoop") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_FUEL_SCOOPS;
    slotType = UTILITY_SYSTEM_1;
    formalName = "Fuel Scoop";
    cost = COST_FUEL_SCOOPS;
    requiredTechLevel = 3;
  } else if (strcmp(equipName, "dockcomp") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER;
    slotType = UTILITY_SYSTEM_2;
    formalName = "Docking Computer";
    cost = COST_DOCKING_COMPUTER;
    requiredTechLevel = 5;
  } else if (strcmp(equipName, "escape") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    slotType = UTILITY_SYSTEM_3;
    formalName = "Escape Pod";
    cost = COST_ESCAPE_POD;
    requiredTechLevel = 5;
  } else if (strcmp(equipName, "cargo") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    slotType = UTILITY_SYSTEM_4;
    formalName = "Cargo Bay Extension";
    cost = COST_CARGO_BAY_EXTENSION;
    requiredTechLevel = 1;
    // Apply the cargo upgrade directly
    if (Cash < cost) {
      printf("\nInsufficient credits to purchase Cargo Bay Extension. "
             "Required: %d, Available: %.1f",
             cost, (float)Cash / 10.0f);
      return 0;
    }

    if (techLevel < requiredTechLevel) {
      printf("\nCargo Bay Extensions not available at this tech level. "
             "Required: %d, Current: %d",
             requiredTechLevel + 1, techLevel + 1);
      return 0;
    }
    // Directly apply the upgrade
    Cash -= cost;
    extern uint16_t HoldSpace;
    HoldSpace += CARGO_BAY_EXTENSION_CAPACITY;
    PlayerShipPtr->attributes.cargoCapacityTons += CARGO_BAY_EXTENSION_CAPACITY;

    // Update equipment mapping
    MapEquipmentIndices(PlayerShipPtr);

    printf("\nCargo Bay Extension installed. New capacity: %d tonnes.",
           PlayerShipPtr->attributes.cargoCapacityTons);
    return 1;  } else if (strcmp(equipName, "pulse") == 0) {
    equipType.weaponType = WEAPON_TYPE_PULSE_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Pulse Laser";
    cost = COST_PULSE_LASER;
    requiredTechLevel = 1;
    damageOutput = 5.0;
  } else if (strcmp(equipName, "beam") == 0) {
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Beam Laser";
    cost = COST_BEAM_LASER;
    requiredTechLevel = 3;
    damageOutput = 7.5;
  } else if (strcmp(equipName, "military") == 0) {
    equipType.weaponType = WEAPON_TYPE_MILITARY_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Military Laser";
    cost = COST_MILITARY_LASER;
    requiredTechLevel = 6;
    damageOutput = 10.0;
  } else if (strcmp(equipName, "mining") == 0) {
    equipType.weaponType = WEAPON_TYPE_MINING_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Mining Laser";
    cost = COST_MINING_LASER;
    requiredTechLevel = 2;
    damageOutput = 3.0;
  } else if (strcmp(equipName, "scanner") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE;
    slotType = UTILITY_SYSTEM_3;
    formalName = "Advanced Scanner";
    cost = COST_SCANNER_UPGRADE;
    requiredTechLevel = 4;
  } else if (strcmp(equipName, "missile") == 0) {
    // For missiles, we just add to the count
    if (PlayerShipPtr->attributes.missilesLoadedHoming >=
        PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY) {
      printf("\nCannot purchase more missiles. All pylons are full.");
      return 0;
    }

    if (Cash < COST_MISSILE_HOMING * 10) {
      printf("\nInsufficient credits to purchase missile. Required: %d, "
             "Available: %.1f",
             COST_MISSILE_HOMING, (float)Cash / 10.0f);
      return 0;
    }

    Cash -= COST_MISSILE_HOMING * 10;
    PlayerShipPtr->attributes.missilesLoadedHoming++;
    printf("\nMissile purchased. Current missile count: %d/%d",
           PlayerShipPtr->attributes.missilesLoadedHoming,
           PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY);
    return 1;
  } else {
    printf("\nUnknown equipment: %s", equipName);
    printf("\nUse 'equip' without parameters to see available equipment.");
    return 0;
  }
  // Attempt to purchase the selected equipment
  bool result =
      PurchaseEquipment(PlayerShipPtr, formalName, slotType, equipType, cost,
                        requiredTechLevel, damageOutput);

  return result;
}

/**
 * Displays the equipment inventory of the player's ship.
 *
 * @param commandArguments Arguments provided to the command (unused)
 * @return 1 if the command was processed successfully
 */
static inline bool do_inventory_display(char *commandArguments) {
  (void)commandArguments; // Mark as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return 0;
  }

  ListEquipmentInventory(PlayerShipPtr);
  return 1;
}

/**
 * Stores equipment from a specified slot into the inventory.
 *
 * @param commandArguments Arguments provided to the command (slot number)
 * @return 1 if the equipment was stored successfully
 */
static inline bool do_store_equipment(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return 0;
  }

  // Check if we're in combat
  if (InCombat) {
    printf("\nCannot modify ship configuration during combat.");
    return 0;
  }

  // Check if a slot number was provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: store <slot_number>");
    printf("\n\nAvailable Equipment Slots:");
    PrintEquipmentSlots(PlayerShipPtr);
    return 0;
  }

  // Parse the slot number
  int slotNumber = atoi(commandArguments);

  // Check if the slot number is valid
  if (slotNumber < 0 || slotNumber >= MAX_EQUIPMENT_SLOTS) {
    printf("\nInvalid slot number. Valid range: 0-%d", MAX_EQUIPMENT_SLOTS - 1);
    return 0;
  }

  // Try to store the equipment
  return RemoveEquipmentToInventory(PlayerShipPtr, slotNumber);
}

/**
 * Equips an item from inventory into a specified slot.
 *
 * @param commandArguments Arguments provided to the command (inventory_index
 * slot_number)
 * @return 1 if the equipment was equipped successfully
 */
static inline bool do_equip_from_inventory(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return 0;
  }

  // Check if we're in combat
  if (InCombat) {
    printf("\nCannot modify ship configuration during combat.");
    return 0;
  }
  // Check if arguments were provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: use <inventory_index> <slot_number>\n");
    printf("Example: use 0 1  (equips item from inventory slot 0 to equipment "
           "slot 1)\n");
    printf("\nUse 'inv' command to view your inventory and 'shipinfo' to see "
           "available slots.\n");
    return 0;
  } // Parse the arguments - we need two numbers: inventory index and slot
  // number
  char arg1[MAX_LEN];
  char arg2[MAX_LEN];
  int invIndex = -1;
  int slotNumber = -1;
  // Extract the arguments
  char *saveptr;
  char *token = safe_strtok(commandArguments, " \t", &saveptr);
  if (token != NULL) {
    snprintf(arg1, MAX_LEN, "%s", token);
    invIndex = atoi(arg1);

    token = safe_strtok(NULL, " \t", &saveptr);
    if (token != NULL) {
      snprintf(arg2, MAX_LEN, "%s", token);
      slotNumber = atoi(arg2);
    } else {
      printf("\nUsage: use <inventory_index> <slot_number>\n");
      printf("Example: use 0 1  (equips item from inventory slot 0 to "
             "equipment slot 1)\n");
      printf("\nUse 'inv' command to view your inventory and 'shipinfo' to see "
             "available slots.\n");
      return 0;
    }
  } // Check if both arguments were provided and are valid
  if (invIndex < 0 || invIndex >= MAX_EQUIPMENT_INVENTORY) {
    printf("\nInvalid inventory index. Valid range: 0-%d\n",
           MAX_EQUIPMENT_INVENTORY - 1);
    return 0;
  }

  if (slotNumber < 0 || slotNumber >= MAX_EQUIPMENT_SLOTS) {
    printf("\nInvalid slot number. Valid range: 0-%d\n",
           MAX_EQUIPMENT_SLOTS - 1);
    return 0;
  }

  // Try to equip the item from inventory
  return EquipFromInventory(PlayerShipPtr, invIndex, slotNumber);
}

// Ship trading commands
static inline bool do_shipyard(char *args) {
  (void)args; // Mark args as unused

  // Check if player is docked at a station
  extern struct NavigationState PlayerNavState;
  extern int PlayerLocationType;

  // Need to be both at a station AND docked
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION ||
      PlayerLocationType != 10) {
    printf("Error: You must be docked at a station to access the shipyard.\n");
    return 0;
  }

  // Get current system info
  extern char CurrentSystemName[20];      // From elite_player_state.h
  extern int CurrentSystemEconomy;        // From elite_player_state.h
  extern uint64_t currentGameTimeSeconds; // From elite_state.h
  extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

  // Display the shipyard
  DisplayShipyard(CurrentSystemName, CurrentSystemEconomy, PlayerShipPtr,
                  currentGameTimeSeconds);

  return 1;
}

static inline bool do_compareship(char *args) {
  // Check if arguments are provided
  if (args == NULL || args[0] == '\0') {
    printf("Error: Please specify a ship to compare with.\n");
    printf("Usage: compareship <shipname>\n");
    return 0;
  }

  // Get player ship
  extern PlayerShip *PlayerShipPtr; // From elite_player_state.h

  // Compare ships
  CompareShips(PlayerShipPtr, (const char *)args);

  return 1;
}

static inline bool do_buyship(char *args) {
  // Check if player is docked at a station
  extern struct NavigationState PlayerNavState;
  extern int PlayerLocationType;

  // Need to be both at a station AND docked
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION ||
      PlayerLocationType != 10) {
    printf("Error: You must be docked at a station to purchase a ship.\n");
    return 0;
  }

  // Check if arguments are provided
  if (args == NULL || args[0] == '\0') {
    printf("Error: Please specify a ship to buy.\n");
    printf("Usage: buyship <ID or shipname> [notrade]\n");
    printf("Example: buyship 1  or  buyship \"Cobra Mk III\"\n");
    return 0;
  }

  // Get current system info and player ship
  extern char CurrentSystemName[20];      // From elite_player_state.h
  extern int CurrentSystemEconomy;        // From elite_player_state.h
  extern uint64_t currentGameTimeSeconds; // From elite_state.h
  extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

  // Parse arguments
  char shipNameOrID[64] = {0};
  bool tradeIn = 1;
  // Copy the first part of the arguments (up to the first space)
  const char *space = strchr(args, ' ');
  if (space != NULL) {
    size_t nameLen = space - args;
    nameLen = (nameLen < 63) ? nameLen : 63;
    snprintf(shipNameOrID, nameLen + 1, "%.*s", (int)nameLen, args);

    // Check for 'notrade' flag in the remaining part
    if (strstr(space + 1, "notrade") != NULL) {
      tradeIn = 0;
    }
  } else {
    // No space, just copy the entire argument
    snprintf(shipNameOrID, sizeof(shipNameOrID), "%s", args);
  }

  // Check if the argument is a number (ID) or a string (ship name)
  char actualShipName[MAX_SHIP_NAME_LENGTH] = {0};
  bool isID = 1;

  // Check if shipNameOrID is a number
  for (size_t i = 0; i < strlen(shipNameOrID); i++) {
    if (!isdigit(shipNameOrID[i])) {
      isID = 0;
      break;
    }
  }

  if (isID) {
    // Convert the ID to an integer
    int shipID = atoi(shipNameOrID);

    // Get the ship name by ID
    if (!GetShipNameByID(CurrentSystemName, CurrentSystemEconomy, shipID,
                         actualShipName, MAX_SHIP_NAME_LENGTH)) {
      printf("Error: Invalid ship ID: %d\n", shipID);
      return 0;
    }
  } else {
    // The argument is a ship name, just copy it
    snprintf(actualShipName, MAX_SHIP_NAME_LENGTH, "%s", shipNameOrID);
  }
  // Buy the new ship
  return BuyNewShip(CurrentSystemName, CurrentSystemEconomy, PlayerShipPtr,
                    (const char *)actualShipName, currentGameTimeSeconds,
                    tradeIn);
}

/**
 * Command handler for the 'upgrade' command
 * Shows available ship upgrades or purchases a specific upgrade
 */
static inline bool do_upgrade(char *commandArguments) {
  return UpgradeCommand(commandArguments);
}

/**
 * Display detailed fuel information for the current ship
 */
static inline bool show_fuel_status(char *commandArguments) {
  // Unused parameter
  (void)commandArguments;

  // Forward declaration for function from elite_player_state.h
  extern void display_ship_fuel_status(void);

  // Call the function that displays detailed fuel information
  display_ship_fuel_status();
  return 1;
}

/**
 * Command to jettison cargo from the ship (discard cargo into space).
 * Usage: jettison <cargo_name> <quantity>
 *        jettison all
 * This will remove the specified cargo from the ship without receiving payment.
 * Using "jettison all" will discard all cargo currently in the ship.
 * Useful in emergencies or when carrying illegal goods and avoiding
 * authorities.
 *
 * @param commandArguments Arguments provided to the command
 * @return 1 if the cargo was successfully jettisoned
 */
static inline bool do_jettison(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return 0;
  }

  if (commandArguments == NULL || strlen(commandArguments) == 0) {
    printf("\nUsage: jettison <cargo_name> <quantity>");
    printf("\nUsage: jettison all");
    printf("\nExample: jettison Food 5");
    return 0;
  }

  // Check if the "all" flag was used
  if (StringCompareIgnoreCase(commandArguments, "all") == 0) {
    // Special case: jettison all cargo
    return JettisonAllCargo(PlayerShipPtr);
  } // Parse the arguments
  char cargoName[MAX_LEN];
  char quantityStr[MAX_LEN];

  // Split the command arguments to get the cargo name
  split_string_at_first_space(commandArguments, cargoName);

  // Get the quantity part
  commandArguments = strip_leading_trailing_spaces(commandArguments);

  // If quantity is not provided, default to 1
  int quantity = 1;
  if (strlen(commandArguments) > 0) {
    snprintf(quantityStr, MAX_LEN, "%s", commandArguments);
    quantity = atoi(quantityStr);
  }
  // Verify quantity is valid
  if (quantity <= 0) {
    printf("\nInvalid quantity. Please specify a positive number.");
    return 0;
  }
  // No need to modify cargo name capitalization since we use case-insensitive
  // comparison The StringCompareIgnoreCase function will handle different
  // capitalizations

  // Find the cargo index in the ShipHold array (needed for synchronization)
  uint16_t cargoIndex = 0;
  bool cargoFound = 0;

  // First, check if the cargo exists in the player's ship
  if (!GetCargoQuantity(PlayerShipPtr, cargoName)) {
    printf("\nError: %s not found in cargo hold.", cargoName);
    return 0;
  }

  // Find the cargo index in the global tradnames array
  for (uint16_t i = 0; i <= LAST_TRADE; i++) {
    if (StringCompareIgnoreCase(tradnames[i], cargoName) == 0) {
      cargoIndex = i;
      cargoFound = 1;
      break;
    }
  }

  if (!cargoFound) {
    printf("\nError: Unable to find cargo in global inventory. Please report "
           "this bug.");
    return 0;
  }

  // Call the JettisonCargo function to remove from player ship
  if (JettisonCargo(PlayerShipPtr, cargoName, quantity)) {
    // Update the global ShipHold array
    if (ShipHold[cargoIndex] >= quantity) {
      ShipHold[cargoIndex] -= quantity;

      // Update HoldSpace if it's measured in tons
      if (Commodities[cargoIndex].units == TONNES_UNIT) {
        HoldSpace += quantity;
      }

      // Synchronize the cargo systems
      SynchronizeCargoSystems(PlayerShipPtr);
      return 1;
    } else {
      printf(
          "\nError: Global cargo quantity mismatch. Please report this bug.");
      // Try to recover by synchronizing
      SynchronizeCargoSystems(PlayerShipPtr);
      return 0;
    }
  } else {
    printf("\nFailed to jettison %s. Check cargo name and quantity.",
           cargoName);
    return 0;
  }
}

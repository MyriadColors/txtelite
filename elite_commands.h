#pragma once

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
#include "platform_compat.h" // For cross-platform file operations
#include <ctype.h>           // For toupper, tolower
#include <math.h>            // For floor, fabs
#include <stdlib.h>          // For atoi, atof
#include <string.h>          // For string operations
#include <time.h>            // For time functions

static inline bool do_tweak_random_native(char *commandArguments) {
  (void)commandArguments; // Mark 's' as unused
  NativeRand ^= 1;
  return true;
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

      print_system_info(Galaxy[syscount], true);
      printf(" (%.1f LY)", (float)d / 10.0f);
    }
  }
  printf("\n");
  return true;
}

static inline bool do_jump(char *commandArguments) {
  uint16_t d;
  PlanetNum dest = find_matching_system_name(commandArguments);

  if (dest == CurrentPlanet) {
    printf("\nBad jump");
    return false;
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
    return false;
  }

  // Check ship energy requirements - jumps require energy
  if (PlayerShipPtr != NULL) {
    // Hyperjump requires at least 20% of max energy
    float energyRequired = PlayerShipPtr->attributes.maxEnergyBanks * 0.2f;

    if (PlayerShipPtr->attributes.energyBanks < energyRequired) {
      printf("\nInsufficient energy for hyperspace jump");
      return false;
    }
    // Consume energy for jump
    PlayerShipPtr->attributes.energyBanks -= energyRequired;

    // Use the new ConsumeFuel function to update both global and ship fuel
    if (!ConsumeFuel((double)fuelNeeded, false)) {
      printf("\nJump failed: Insufficient fuel");
      return false;
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
  print_system_info(Galaxy[CurrentPlanet], false);
  return true;
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

  return false;
}

static inline bool do_planet_info_display(char *commandArguments) {
  PlanetNum dest = find_matching_system_name(commandArguments);
  if (dest < GAL_SIZE) { // Check if a valid planet was found
    print_system_info(Galaxy[dest], false);
  } else {
    printf("\nPlanet not found: %s", commandArguments);
    return false;
  }
  return true;
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
    return false;
  }

  HoldSpace = a - t;
  printf("\nHold space set to %u. Available: %u tonnes.", a, HoldSpace);
  return true;
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
    return false;
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
  return true;
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
    return false;
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
  return true;
}

static inline bool do_fuel(char *commandArguments) {
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: fuel <amount>");
    return false;
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
  return true;
}

static inline bool do_cash(char *commandArguments) {
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: cash <amount>");
    return false;
  }
  int a = (int)(10 * atof(commandArguments)); // Amount is in tenths of credits
  Cash += (long)a;

  if (a != 0) {
    printf("\nCash adjusted by %.1f. Current cash: %.1f CR.", (float)a / 10.0f,
           (float)Cash / 10.0f);
    return true;
  }

  printf("Number not understood for cash command.");
  return false;
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
  return true;
}

static inline bool do_quit(char *commandArguments) {
  (void)(commandArguments);
  printf("\nExiting Text Elite. Goodbye!\n");
  exit(ExitStatus);
  // This line will not be reached if ExitStatus leads to a successful exit.
  // It's here to satisfy the function signature if exit() somehow didn't
  // terminate.
  return true;
}

/**
 * Resets the game state to a new game, either with a custom seed or the default
 * seed. Usage: reset [seed] If no seed is provided, uses the default seed
 * "54321"
 *
 * @param commandArguments Optional seed to use for the new game
 * @return true if the reset was successful
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

  return true;
}

static inline bool do_help(char *commandArguments) {
  // If specific help is requested for a command, show detailed help
  if (commandArguments && strlen(commandArguments) > 0) {
    // Properly strip spaces from the command argument
    commandArguments = strip_leading_trailing_spaces(commandArguments);

    char command[MAX_LEN];
    snprintf(command, MAX_LEN, "%s", commandArguments);

    // Convert to lowercase for case-insensitive matching
    for (char *p = command; *p; ++p)
      *p = tolower(*p);

    // Ship-related commands
    if (strcmp(command, "ship") == 0) {
      printf("\nSHIP - Display basic ship status information");
      printf("\n  Shows hull integrity, energy, fuel, and cargo capacity");
      return true;
    } else if (strcmp(command, "shipinfo") == 0) {
      printf("\nSHIPINFO - Display detailed ship information");
      printf("\n  Shows comprehensive information about your ship's systems,");
      printf("\n  equipment, and cargo hold contents");
      return true;
    } else if (strcmp(command, "repair") == 0) {
      printf("\nREPAIR - Repair your ship's hull damage");
      printf("\n  This command will repair your ship to 100%% hull integrity");
      printf("\n  Cost is 10 credits per hull point repaired");
      printf("\n  Note: You must be docked at a station to repair your ship");
      return true;
    } else if (strcmp(command, "equip") == 0) {
      printf("\nEQUIP [equipment] - Purchase and install ship equipment");
      printf("\n  Without parameters: Lists all available equipment");
      printf("\n  [equipment]: The specific equipment item to purchase");
      printf("\n  Available equipment types:");
      printf("\n    ecm      - Electronic Counter Measures (600 CR)");
      printf("\n    fuelscoop - Fuel Scoop for collecting fuel from stars (525 "
             "CR)");
      printf(
          "\n    dockcomp - Docking Computer for automated docking (1500 CR)");
      printf("\n    escape   - Escape Pod for emergency escape (1000 CR)");
      printf("\n    energy   - Extra Energy Unit for more energy capacity "
             "(1500 CR)");
      printf("\n    ebomb    - Energy Bomb for emergency defense (2500 CR)");
      printf(
          "\n    cargo    - Cargo Bay Extension for +4 tons capacity (400 CR)");
      printf("\n    pulse    - Pulse Laser for basic combat (400 CR)");
      printf("\n    beam     - Beam Laser for improved combat (1000 CR)");
      printf("\n    military - Military Laser for maximum firepower (2500 CR)");
      printf("\n    mining   - Mining Laser for resource extraction (800 CR)");
      printf(
          "\n    scanner  - Scanner Upgrade for improved detection (700 CR)");
      printf("\n    missile  - Homing Missile for one-shot attacks (300 CR)");
      printf("\n  Example: equip beam");
      printf("\n  Note: You must be docked at a station to purchase equipment");
      printf("\n        Equipment availability depends on the system's tech "
             "level");
      return true;
    } else if (strcmp(command, "fuel") == 0 || strcmp(command, "f") == 0) {
      printf("\nFUEL <amount> - Purchase fuel for your ship");
      printf("\n  <amount> - Amount of fuel to buy in light years");
      printf("\n  Example: fuel 2.5");
      printf("\n  Note: You must be docked at a station to buy fuel.");
      printf("\n        Fuel costs %.1f credits per 0.1 LY unit for your "
             "current ship",
             (float)GetFuelCost() / 10.0f);
      return true;
    } else if (strcmp(command, "fuelinfo") == 0) {
      printf("\nFUELINFO - Display detailed fuel information for your ship");
      printf(
          "\n  Shows current fuel level, maximum capacity, consumption rate,");
      printf("\n  estimated range, and refill cost based on your ship's "
             "specifications");
      printf("\n  This command has no parameters");
      return true;
    }

    // Trading commands
    else if (strcmp(command, "buy") == 0 || strcmp(command, "b") == 0) {
      printf("\nBUY <good> <amount> - Purchase goods from the market");
      printf("\n  <good>   - Type of trade good (e.g., Food, Computers)");
      printf("\n  <amount> - Quantity to buy (default: 1)");
      printf("\n  Example: buy Food 5");
      printf("\n  Note: You must be docked at a station with a market to buy "
             "goods.");
      return true;
    }
    if (strcmp(command, "sell") == 0 || strcmp(command, "s") == 0) {
      printf("\nSELL <good> <amount> - Sell goods to the market");
      printf("\n  <good>   - Type of trade good (e.g., Food, Computers)");
      printf("\n  <amount> - Quantity to sell (default: 1)");
      printf("\n  Example: sell Computers 3");
      printf("\n  Note: You must be docked at a station with a market to sell "
             "goods.");
      return true;
    }

    if (strcmp(command, "jettison") == 0 || strcmp(command, "j") == 0) {
      printf("\nJETTISON <good> <amount> or JETTISON ALL - Discard cargo into "
             "space");
      printf("\n  <good>   - Type of trade good to jettison (e.g., Food, "
             "Computers)");
      printf("\n  <amount> - Quantity to jettison (default: 1)");
      printf("\n  ALL      - Special flag to jettison all cargo at once");
      printf("\n  Examples: jettison Food 5");
      printf("\n            jettison all");
      printf("\n  Note: Jettisoned cargo is lost permanently with no payment "
             "received.");
      printf("\n        Useful in emergencies or when carrying illegal goods "
             "and avoiding authorities.");
      return true;
    }

    // Navigation commands
    if (strcmp(command, "jump") == 0 || strcmp(command, "j") == 0) {
      printf("\nJUMP <planetname> - Jump to another star system");
      printf("\n  <planetname> - Name of the destination system");
      printf("\n  Example: jump Lave");
      printf("\n  Note: Requires fuel equal to the distance in light years.");
      printf("\n        Use 'local' to see systems within jump range.");
      return true;
    }

    if (strcmp(command, "local") == 0 || strcmp(command, "l") == 0) {
      printf("\nLOCAL - List star systems within jump range");
      printf("\n  Systems marked with * are within current fuel range.");
      printf("\n  Systems marked with - are within maximum fuel capacity but "
             "require refueling.");
      printf("\n  Distances are shown in light years (LY).");
      return true;
    }

    if (strcmp(command, "galhyp") == 0 || strcmp(command, "g") == 0) {
      printf("\nGALHYP - Perform a galactic hyperspace jump");
      printf("\n  Jumps to the next galaxy (1-8).");
      printf("\n  No fuel is required for this special jump.");
      return true;
    }
    // Star system navigation commands
    if (strcmp(command, "system") == 0 || strcmp(command, "sys") == 0) {
      printf("\nSYSTEM - Displays detailed information about the current star "
             "system");
      printf("\n  Shows all celestial bodies, stations, their locations, and "
             "travel times.");
      printf("\n  Note: This command (formerly also available as 'scan') scans "
             "the system");
      printf(
          "\n        for points of interest and costs 1 minute of game time.");
      return true;
    }

    if (strcmp(command, "travel") == 0 || strcmp(command, "t") == 0) {
      printf("\nTRAVEL [destination] - Travel within the current star system");
      printf("\n  Without parameters: Lists all available destinations.");
      printf("\n  [destination]: The location to travel to, using the "
             "numbering system:");
      printf("\n    0       - Travel to the central star");
      printf("\n    1-8     - Travel to a planet (number depends on system)");
      printf("\n    1.1-8.5 - Travel to a station (format: planet.station)");
      printf("\n    N       - Travel to the Nav Beacon");
      printf("\n  Example: travel 2    - Travel to the second planet");
      printf("\n  Example: travel 1.3  - Travel to the third station orbiting "
             "the first planet");
      printf("\n  Example: travel N    - Travel to the Nav Beacon");
      printf("\n  Note: Travel consumes game time based on distance and energy "
             "based on distance.");
      printf("\n        Energy requirements are calculated at a rate of 1 "
             "energy unit per 0.1 AU.");
      printf(
          "\n        Fuel is also consumed at a rate of 0.025 liters per AU.");
      return true;
    }
    if (strcmp(command, "dock") == 0 || strcmp(command, "d") == 0) {
      printf("\nDOCK - Dock with the current station");
      printf("\n  Must be at a station location before docking.");
      printf("\n  Use 'travel' to navigate to a station first.");
      printf(
          "\n  Docking provides access to market and other station services.");
      printf("\n  No parameters required.");
      return true;
    }

    if (strcmp(command, "land") == 0) {
      printf("\nLAND - Land on a planet surface");
      printf("\n  Allows you to land on a planet when your ship is at a planet "
             "location.");
      printf("\n  You must be at a planet before landing.");
      printf("\n  Use 'travel' to navigate to a planet first.");
      printf(
          "\n  Landing provides access to the planet's market and services.");
      printf("\n  No parameters required.");
      return true;
    }

    // Ship trading commands
    if (strcmp(command, "shipyard") == 0) {
      printf("\nSHIPYARD - View ships available for purchase");
      printf("\n  Shows a list of ships available at the current station.");
      printf("\n  Displays hull strength, energy, cargo capacity, and price.");
      printf("\n  Includes your current ship's trade-in value.");
      printf("\n  You must be docked at a station to use this command.");
      printf("\n  No parameters required.");
      return true;
    }

    if (strcmp(command, "compareship") == 0) {
      printf("\nCOMPARESHIP <shipname> - Compare your ship with another ship "
             "type");
      printf("\n  Displays a side-by-side comparison of ship specifications.");
      printf("\n  Shows differences in hull, energy, shields, cargo, etc.");
      printf("\n  Usage: compareship <shipname> (e.g., 'compareship Viper')");
      printf("\n  Works anywhere, docking not required.");
      return true;
    }
    if (strcmp(command, "buyship") == 0) {
      printf("\nBUYSHIP <ID or shipname> [notrade] - Purchase a new ship");
      printf("\n  Buys a new ship from the current station's shipyard.");
      printf("\n  <ID> - The ship ID number shown in the shipyard list");
      printf(
          "\n  <shipname> - The name of the ship (for backward compatibility)");
      printf("\n  By default, trades in your current ship for a credit.");
      printf("\n  Use 'notrade' flag to buy without trading in (e.g., 'buyship "
             "1 notrade').");
      printf("\n  Equipment and cargo are transferred when possible.");
      printf("\n  You must be docked at a station to use this command.");
      printf("\n  Examples: 'buyship 1' or 'buyship \"Cobra Mk III\"'");
      return true;
    }

    // Market commands
    if (strcmp(command, "mkt") == 0 || strcmp(command, "m") == 0) {
      printf("\nMKT - Display market information");
      printf("\n  Shows current market prices, cash, fuel level, and cargo "
             "status.");
      printf("\n  No parameters required.");
      printf("\n  Note: Market prices vary between systems based on economy "
             "type.");
      return true;
    }

    if (strcmp(command, "fuel") == 0 || strcmp(command, "f") == 0) {
      printf("\nFUEL <amount> - Purchase fuel");
      printf("\n  <amount> - Amount of fuel to buy in light years");
      printf("\n  Example: fuel 7");
      printf("\n  Note: Your maximum fuel capacity is 7 light years.");
      return true;
    }

    // Cargo and Money commands
    if (strcmp(command, "hold") == 0 || strcmp(command, "h") == 0) {
      printf("\nHOLD <amount> - Set cargo hold capacity");
      printf("\n  <amount> - Total cargo hold space in tonnes");
      printf("\n  Example: hold 20");
      printf("\n  Note: Cannot reduce hold space below current cargo volume.");
      return true;
    }

    if (strcmp(command, "cash") == 0 || strcmp(command, "c") == 0) {
      printf("\nCASH <+/-amount> - Adjust cash balance");
      printf("\n  <+/-amount> - Amount to add or subtract from cash balance");
      printf("\n  Example: cash +100.0  - Add 100 credits");
      printf("\n  Example: cash -50.5   - Subtract 50.5 credits");
      printf("\n  Note: This is a debug command for testing purposes.");
      return true;
    }

    // Game management commands
    if (strcmp(command, "save") == 0) {
      printf("\nSAVE [description] - Save the current game state");
      printf("\n  [description] - Optional description of the save (e.g., 'At "
             "Lave')");
      printf("\n  Example: save Trading at Lave");
      printf("\n  Note: Save files are timestamped and stored in the 'saves' "
             "directory.");
      return true;
    }

    if (strcmp(command, "load") == 0) {
      printf("\nLOAD - List and load saved games");
      printf("\n  Shows a list of available save files, sorted by most recent "
             "first.");
      printf("\n  Enter the number of the save file to load when prompted.");
      printf("\n  Note: Loading a save will discard your current game state.");
      return true;
    }

    if (strcmp(command, "reset") == 0) {
      printf("\nRESET [seed] - Restart the game with an optional random seed");
      printf("\n  Without parameters: Reinitializes the game with default seed "
             "54321");
      printf("\n  [seed]: A positive integer to use as the random seed");
      printf("\n  Example: reset, reset 12345");
      printf("\n  Note: Resetting will discard your current game state and "
             "begin a new game.");
      return true;
    }

    if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
      printf("\nQUIT - Exit the game");
      printf("\n  Exits the game without saving. Use 'save' first to preserve "
             "your progress.");
      return true;
    }

    // Debug commands
    if (strcmp(command, "rand") == 0) {
      printf("\nRAND - Toggle random number generator");
      printf("\n  Switches between native and portable RNG implementations.");
      printf("\n  This is a debug command for testing purposes.");
      return true;
    }

    if (strcmp(command, "sneak") == 0) {
      printf(
          "\nSNEAK <planetname> - Jump to another system without using fuel");
      printf("\n  <planetname> - Name of the destination system");
      printf("\n  Example: sneak Lave");
      printf("\n  Note: This is a debug command for testing purposes.");
      return true;
    }

    if (strcmp(command, "info") == 0 || strcmp(command, "i") == 0) {
      printf("\nINFO <planetname> - Display information about a system");
      printf("\n  <planetname> - Name of the system to get information about");
      printf("\n  Example: info Lave");
      printf("\n  Shows economy, government, tech level, and other system "
             "details.");
      return true;
    }

    if (strcmp(command, "compare") == 0) {
      printf("\nCOMPARE - Compare markets across different stations in the "
             "system");
      printf("\n  Shows price differences and profit opportunities between "
             "stations.");
      printf(
          "\n  Lists all stations in the system with their distance from you.");
      printf("\n  Highlights best commodities to buy or sell at each station.");
      printf("\n  Shows estimated travel times to other stations.");
      printf("\n  Note: You must be docked at a station to use this command.");
      return true;
    }

    if (strcmp(command, "mkt") == 0 || strcmp(command, "m") == 0) {
      printf("\nMKT - Display market information");
      printf("\n  Shows current market prices, cash, fuel level, and cargo "
             "status.");
      printf("\n  No parameters required.");
      printf("\n  Note: Market prices vary between systems based on economy "
             "type.");
      return true;
    }

    if (strcmp(command, "fuel") == 0 || strcmp(command, "f") == 0) {
      printf("\nFUEL <amount> - Purchase fuel");
      printf("\n  <amount> - Amount of fuel to buy in light years");
      printf("\n  Example: fuel 7");
      printf("\n  Note: Your maximum fuel capacity is 7 light years.");
      return true;
    }

    // Cargo and Money commands
    if (strcmp(command, "hold") == 0 || strcmp(command, "h") == 0) {
      printf("\nHOLD <amount> - Set cargo hold capacity");
      printf("\n  <amount> - Total cargo hold space in tonnes");
      printf("\n  Example: hold 20");
      printf("\n  Note: Cannot reduce hold space below current cargo volume.");
      return true;
    }

    if (strcmp(command, "cash") == 0 || strcmp(command, "c") == 0) {
      printf("\nCASH <+/-amount> - Adjust cash balance");
      printf("\n  <+/-amount> - Amount to add or subtract from cash balance");
      printf("\n  Example: cash +100.0  - Add 100 credits");
      printf("\n  Example: cash -50.5   - Subtract 50.5 credits");
      printf("\n  Note: This is a debug command for testing purposes.");
      return true;
    }

    // Game management commands
    if (strcmp(command, "save") == 0) {
      printf("\nSAVE [description] - Save the current game state");
      printf("\n  [description] - Optional description of the save (e.g., 'At "
             "Lave')");
      printf("\n  Example: save Trading at Lave");
      printf("\n  Note: Save files are timestamped and stored in the 'saves' "
             "directory.");
      return true;
    }

    if (strcmp(command, "load") == 0) {
      printf("\nLOAD - List and load saved games");
      printf("\n  Shows a list of available save files, sorted by most recent "
             "first.");
      printf("\n  Enter the number of the save file to load when prompted.");
      printf("\n  Note: Loading a save will discard your current game state.");
      return true;
    }

    if (strcmp(command, "reset") == 0) {
      printf("\nRESET [seed] - Restart the game with an optional random seed");
      printf("\n  Without parameters: Reinitializes the game with default seed "
             "54321");
      printf("\n  [seed]: A positive integer to use as the random seed");
      printf("\n  Example: reset, reset 12345");
      printf("\n  Note: Resetting will discard your current game state and "
             "begin a new game.");
      return true;
    }

    if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
      printf("\nQUIT - Exit the game");
      printf("\n  Exits the game without saving. Use 'save' first to preserve "
             "your progress.");
      return true;
    }

    // Debug commands
    if (strcmp(command, "rand") == 0) {
      printf("\nRAND - Toggle random number generator");
      printf("\n  Switches between native and portable RNG implementations.");
      printf("\n  This is a debug command for testing purposes.");
      return true;
    }

    if (strcmp(command, "sneak") == 0) {
      printf(
          "\nSNEAK <planetname> - Jump to another system without using fuel");
      printf("\n  <planetname> - Name of the destination system");
      printf("\n  Example: sneak Lave");
      printf("\n  Note: This is a debug command for testing purposes.");
      return true;
    }

    if (strcmp(command, "info") == 0 || strcmp(command, "i") == 0) {
      printf("\nINFO <planetname> - Display information about a system");
      printf("\n  <planetname> - Name of the system to get information about");
      printf("\n  Example: info Lave");
      printf("\n  Shows economy, government, tech level, and other system "
             "details.");
      return true;
    }

    if (strcmp(command, "compare") == 0) {
      printf("\nCOMPARE - Compare markets across different stations in the "
             "system");
      printf("\n  Shows price differences and profit opportunities between "
             "stations.");
      printf(
          "\n  Lists all stations in the system with their distance from you.");
      printf("\n  Highlights best commodities to buy or sell at each station.");
      printf("\n  Shows estimated travel times to other stations.");
      printf("\n  Note: You must be docked at a station to use this command.");
      return true;
    }

    // Equipment and inventory commands
    if (strcmp(command, "inv") == 0) {
      printf("\nINV - Display equipment inventory");
      printf("\n  Shows all equipment items stored in your ship's inventory.");
      printf("\n  Each item is shown with its inventory slot index for use "
             "with the 'use' command.");
      return true;
    }

    if (strcmp(command, "store") == 0) {
      printf("\nSTORE <slot_number> - Remove equipment and store in inventory");
      printf("\n  <slot_number> - The equipment slot to remove equipment from");
      printf("\n  Example: store 0");
      printf("\n  Note: Use 'shipinfo' to see your equipment slots and what's "
             "installed in them.");
      return true;
    }

    if (strcmp(command, "use") == 0) {
      printf(
          "\nUSE <inventory_index> <slot_number> - Equip item from inventory");
      printf("\n  <inventory_index> - The inventory slot containing the "
             "equipment to use");
      printf("\n  <slot_number> - The equipment slot to install the equipment "
             "into");
      printf("\n  Example: use 2 1");
      printf("\n  Note: Equipment can only be installed in compatible slots.");
      printf("\n        Use 'inv' to see your inventory and 'shipinfo' to see "
             "slots.");
      return true;
    }
    // If command not recognized, show general help
    printf("\nUnknown command: %s", command);
    printf("\nUse 'help' without parameters to see all available commands.");
    return true;
  }
  // Display general help categories	printf("\n=== TXTELITE COMMAND REFERENCE
  // ===");
  printf("\n\nTRADING COMMANDS:");
  printf("\n  buy   <good> <amount>   - Buy goods");
  printf("\n  sell  <good> <amount>   - Sell goods");
  printf("\n  jettison <good> <amount> - Discard goods into space");
  printf("\n  jettison all            - Discard all cargo at once");
  printf("\n  mkt                     - Show current market prices, fuel, and "
         "cash");
  printf("\n  compare                 - Compare markets across stations in the "
         "system");
  printf("\n\nINTERSTELLAR NAVIGATION:");
  printf("\n  jump  <planetname>      - Jump to planet (uses fuel)");
  printf("\n  fuel  <amount>          - Buy amount Light Years of fuel");
  printf("\n  galhyp                  - Jump to the next galaxy");
  printf("\n  local                   - List systems within 7 light years");
  printf("\n  info  <planetname>      - Display information about a system");
  printf("\n\nSTAR SYSTEM NAVIGATION:");
  printf("\n  system                  - Scan system for detailed information "
         "and points of interest");
  printf("\n  travel [destination]    - List destinations or travel within the "
         "system (uses energy)");
  printf("\n  dock                    - Dock with a station if at a station "
         "location");
  printf(
      "\n  land                    - Land on a planet if at a planet location");
  printf("\n\nCARGO AND MONEY:");
  printf("\n  hold  <amount>          - Set total cargo hold space in tonnes");
  printf("\n  cash  <+/-amount>       - Adjust cash (e.g., cash +100.0 or cash "
         "-50.5)");
  printf("\n\nSHIP MANAGEMENT:");
  printf("\n  ship                    - Display basic ship status information");
  printf("\n  shipinfo                - Display detailed ship information");
  printf(
      "\n  repair                  - Repair ship's hull damage (when docked)");
  printf("\n  equip [item]            - Purchase and install ship equipment "
         "(ECM, fuel scoop, etc.)");
  printf("\n  inv                     - Display stored equipment items in your "
         "ship's inventory");
  printf("\n  store <slot_number>     - Remove equipment from a slot and store "
         "it in inventory");
  printf("\n  use <inv_idx> <slot>    - Install equipment from inventory into "
         "a ship slot");
  printf("\n  shipyard                - View ships available for purchase at "
         "the station");
  printf(
      "\n  compareship <shipname>  - Compare your ship with another ship type");
  printf("\n  buyship <ID or shipname> - Purchase a new ship (ID from shipyard "
         "list)");
  printf("\n  upgrade [ID] [quantity] - View and purchase ship upgrades (hull, "
         "shields, etc.)");
  printf("\n\nGAME MANAGEMENT:");
  printf(
      "\n  save  [description]     - Save the game with optional description");
  printf(
      "\n  load  [filename]        - List save games or load a specific save");
  printf("\n  reset [seed]            - Reset the game with an optional seed "
         "(default: 54321)");
  printf("\n  quit                    - Exit the game");

  printf("\n\nDEBUG COMMANDS:");
  printf("\n  sneak <planetname>      - Jump to planet (no fuel cost, debug)");
  printf("\n  rand                    - Toggle RNG between native and portable "
         "(debug)");

  printf("\n\nFor detailed help on any command, type 'help <command>'");
  printf("\nAbbreviations allowed for most commands (e.g., b fo 5 for Buy Food "
         "5, m for mkt).\n");
  return true;
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
    return false;
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
    return false;
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
    bool headerValid = false;

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

  return false;
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
    return false;
  }

  // Validate pointer to PlanSys data
  if (!CurrentStarSystem->planSys) {
    printf("\nError: Planet system data not available.");
    return false;
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
          fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);
      uint32_t timeToPlanet = calculate_travel_time(
          PlayerNavState.distanceFromStar, planet->orbitalDistance);
      double energyToPlanet = calculate_travel_energy_requirement(distToPlanet);
      double fuelToPlanet = calculate_travel_fuel_requirement(distToPlanet);
      printf("\n  %d. %s (%.2f AU from star, %.2f AU away, %u min travel, %.1f "
             "energy, %.3f fuel L required)",
             i + 1, planet->name, planet->orbitalDistance, distToPlanet,
             timeToPlanet / 60, energyToPlanet,
             fuelToPlanet); // Planet type and physical characteristics
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

        bool hasValidStations = false;
        for (uint8_t j = 0; j < planet->numStations; j++) {
          Station *station = planet->stations[j];
          if (!station)
            continue; // Skip NULL stations

          hasValidStations = true; // Station type information
          const char *stationTypes[] = {"Orbital", "Coriolis", "Ocellus"};
          double stationDistAbsolute =
              planet->orbitalDistance + station->orbitalDistance;
          double distToStation =
              fabs(PlayerNavState.distanceFromStar - stationDistAbsolute);
          uint32_t timeToStation = calculate_travel_time(
              PlayerNavState.distanceFromStar, stationDistAbsolute);
          double energyToStation =
              calculate_travel_energy_requirement(distToStation);
          double fuelToStation =
              calculate_travel_fuel_requirement(distToStation);

          printf("\n     %d.%d. %s (%.3f AU from planet, %.2f AU away, %u min "
                 "travel, %.1f energy required, %.3f fuel L required)",
                 i + 1, j + 1, station->name, station->orbitalDistance,
                 distToStation, timeToStation / 60, energyToStation,
                 fuelToStation);

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
                                CurrentStarSystem->navBeaconDistance);
  uint32_t timeToNavBeacon = calculate_travel_time(
      PlayerNavState.distanceFromStar, CurrentStarSystem->navBeaconDistance);
  double energyToNavBeacon =
      calculate_travel_energy_requirement(distToNavBeacon);
  double fuelToNavBeacon = calculate_travel_fuel_requirement(distToNavBeacon);
  printf("\n\nNav Beacon: %.2f AU from star (%.2f AU away, %u min travel, %.1f "
         "energy required, %.3f fuel L required)",
         CurrentStarSystem->navBeaconDistance, distToNavBeacon,
         timeToNavBeacon / 60, energyToNavBeacon, fuelToNavBeacon);
  printf("\n  Travel code: N");

  // Current player location
  printf("\n\nCurrent location: %s (%.2f AU from star)", locBuffer,
         PlayerNavState.distanceFromStar);
  // Star distance and travel time (from current location)
  double distToStar = PlayerNavState.distanceFromStar;
  uint32_t timeToStar =
      calculate_travel_time(PlayerNavState.distanceFromStar, 0.0);
  double energyToStar = calculate_travel_energy_requirement(distToStar);
  double fuelToStar = calculate_travel_fuel_requirement(distToStar);
  printf("\nDistance to Star (%s): %.2f AU, %u min travel, %.1f energy "
         "required, %.3f fuel L required",
         CurrentStarSystem->centralStar.name, distToStar, timeToStar / 60,
         energyToStar, fuelToStar);
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

  return true;
}

// Lists available destinations and allows travel within the system
static inline bool do_travel(char *commandArguments) {
  // Check if star system data is properly initialized
  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return false;
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
    return true;
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
    return false;
  }

  // Check for Nav Beacon special case
  if (destStr[0] == 'N' ||
      destStr[0] == 'n') { // Check if already at Nav Beacon
    if (PlayerNavState.currentLocationType == CELESTIAL_NAV_BEACON) {
      printf("\nAlready at Nav Beacon.");
      return true;
    }
    // Calculate energy requirement
    double distanceDelta = fabs(PlayerNavState.distanceFromStar -
                                CurrentStarSystem->navBeaconDistance);
    double energyRequired = calculate_travel_energy_requirement(distanceDelta);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to Nav Beacon... (Energy required: %.1f units, Fuel "
           "required: %.3f liters)",
           energyRequired, fuelRequired);

    // Pass a dummy non-NULL pointer for consistency with the function signature
    void *dummy = &CurrentStarSystem; // Using any valid address as a dummy
    bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                      CELESTIAL_NAV_BEACON, dummy);
    if (result) {
      printf("\nArrived at Nav Beacon (%.2f AU from star)",
             PlayerNavState.distanceFromStar);
      return true;
    } else {
      printf("\nFailed to travel to Nav Beacon.");
      return false;
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
        return false;
      }
    }

    for (char *p = dotPos + 1; *p; p++) {
      if (!isdigit((unsigned char)*p)) {
        printf("\nInvalid station number: %s. Must be a number.", dotPos + 1);
        return false;
      }
    }

    primaryIndex = atoi(destStr);
    secondaryIndex = atoi(dotPos + 1);

    // Validate index ranges
    if (primaryIndex <= 0) {
      printf("\nInvalid planet number: %d. Must be a positive number.",
             primaryIndex);
      return false;
    }

    if (secondaryIndex <= 0) {
      printf("\nInvalid station number: %d. Must be a positive number.",
             secondaryIndex);
      return false;
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
          return false;
        }
      }
      primaryIndex = atoi(destStr);

      if (primaryIndex < 0) {
        printf(
            "\nInvalid destination number: %d. Must be a non-negative number.",
            primaryIndex);
        return false;
      }
    }
  }

  // Special case for star (index 0)
  if (primaryIndex == 0) { // Check if already at star
    if (PlayerNavState.currentLocationType == CELESTIAL_STAR) {
      printf("\nAlready at %s.", CurrentStarSystem->centralStar.name);
      return true;
    }
    // Calculate energy requirement
    double distanceDelta =
        PlayerNavState
            .distanceFromStar; // Distance to star is just current distance
    double energyRequired = calculate_travel_energy_requirement(distanceDelta);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to %s... (Energy required: %.1f units, Fuel required: "
           "%.3f liters)",
           CurrentStarSystem->centralStar.name, energyRequired, fuelRequired);
    bool result =
        travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_STAR,
                            &CurrentStarSystem->centralStar);
    if (result) {
      printf("\nArrived at %s (0.00 AU from star)",
             CurrentStarSystem->centralStar.name);
      return true;
    } else {
      printf("\nFailed to travel to %s.", CurrentStarSystem->centralStar.name);
      return false;
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
    return false;
  }

  Planet *planet = &CurrentStarSystem->planets[primaryIndex];
  if (!planet) {
    printf("\nError: Invalid planet data for planet %d.", primaryIndex + 1);
    return false;
  }

  // If no secondary index, travel to planet
  if (secondaryIndex == -1) { // Check if already at this planet
    if (PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
        PlayerNavState.currentLocation.planet == planet) {
      printf("\nAlready at %s.", planet->name);
      return true;
    }
    // Calculate energy requirement
    double distanceDelta =
        fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);
    double energyRequired = calculate_travel_energy_requirement(distanceDelta);
    double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

    printf("\nTravelling to %s... (Energy required: %.1f units, Fuel required: "
           "%.3f liters)",
           planet->name, energyRequired, fuelRequired);
    bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                      CELESTIAL_PLANET, planet);
    if (result) {
      printf("\nArrived at %s (%.2f AU from star)", planet->name,
             PlayerNavState.distanceFromStar);
      return true;
    } else {
      printf("\nFailed to travel to %s.", planet->name);
      return false;
    }
  }

  // Adjust for 1-based indexing for stations
  secondaryIndex--;

  // Check if station index is valid
  if (secondaryIndex < 0 || secondaryIndex >= planet->numStations) {
    printf("\nInvalid station. Planet %s has %d stations (numbered 1 to %d).",
           planet->name, planet->numStations, planet->numStations);
    return false;
  }

  Station *station = planet->stations[secondaryIndex];
  if (!station) {
    printf("\nError: Station data not available for station %d of planet %s.",
           secondaryIndex + 1, planet->name);
    return false;
  }
  // Check if already at this station
  if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
      PlayerNavState.currentLocation.station == station) {
    printf("\nAlready at %s.", station->name);
    return true;
  }
  // Calculate energy requirement
  double stationDistance = planet->orbitalDistance + station->orbitalDistance;
  double distanceDelta =
      fabs(PlayerNavState.distanceFromStar - stationDistance);
  double energyRequired = calculate_travel_energy_requirement(distanceDelta);
  double fuelRequired = calculate_travel_fuel_requirement(distanceDelta);

  printf("\nTravelling to %s... (Energy required: %.1f units, Fuel required: "
         "%.3f liters)",
         station->name, energyRequired, fuelRequired);
  bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState,
                                    CELESTIAL_STATION, station);
  if (result) {
    printf("\nArrived at %s (%.2f AU from star)", station->name,
           PlayerNavState.distanceFromStar);
    return true;
  } else {
    printf("\nFailed to travel to %s.", station->name);
    return false;
  }
}

// Docks with a station if at a station location
static inline bool do_dock(char *commandArguments) {
  (void)(commandArguments); // Unused parameter

  // Validate star system data
  if (!CurrentStarSystem) {
    printf("\nError: Star system data not available. System might not be "
           "properly initialized.");
    return false;
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
    bool stationsFound = false;
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
          stationsFound = true;
        }
      }
    }

    if (!stationsFound) {
      printf("\n  No stations within 1 AU. Use 'scan' to find all stations in "
             "the system.");
    }

    return false;
  }

  // Validate station data
  Station *station = PlayerNavState.currentLocation.station;
  if (!station) {
    printf("\nError: Station data not available. Cannot complete docking "
           "procedure.");
    return false;
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

  return true;
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
    return false;
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
    bool planetsFound = false;
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
        planetsFound = true;
      }
    }

    if (!planetsFound) {
      printf("\n  No planets within 1 AU. Use 'scan' to find all planets in "
             "the system.");
    }

    return false;
  }

  // At this point, we're at a planet and can land
  Planet *planet = PlayerNavState.currentLocation.planet;

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
      planet->planetaryMarket.isInitialized = true;
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

  return true;
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
    return false;
  }

  MarketType baseMarketToCompare; // Changed MarketInfo to MarketType
  char baseLocationName[MAX_LEN];
  bool isPlanetBase = false; // Initialize isPlanetBase

  // Determine the base market for comparison
  if (PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
      PlayerNavState.currentLocation.planet) {
    Planet *currentPlanet = PlayerNavState.currentLocation.planet;
    if (!currentPlanet) {
      printf("\nError: Current planet data is invalid for comparison.");
      return false;
    }

    snprintf(baseLocationName, MAX_LEN, "%s", currentPlanet->name);
    isPlanetBase = true;

    // Ensure the planetary market is initialized and up-to-date.
    // UpdatePlanetaryMarket handles both initialization and updates.
    // Use local market data since we're at the planet
    baseMarketToCompare = LocalMarket;
  } else if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
             PlayerNavState.currentLocation.station) {
    Station *currentStation = PlayerNavState.currentLocation.station;
    if (!currentStation) {
      printf("\nError: Current station data is invalid for comparison.");
      return false;
    }
    snprintf(baseLocationName, MAX_LEN, "%s", currentStation->name);
    isPlanetBase = false;

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
    return false;
  }

  printf("\n=== MARKET COMPARISON ===");
  printf("\nBase location: %s", baseLocationName);

  bool foundStationsToCompare = false;
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

      foundStationsToCompare = true;
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

  return true;
}

static inline bool do_ship_status(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return false;
  }

  // Note: We don't synchronize fuel here as the ship's fuel value should be
  // more precise and is the source of truth after travel operations

  // Display basic ship information
  printf("\n=== Ship Status: %s (%s) ===", PlayerShipPtr->shipName,
         PlayerShipPtr->shipClassName);

  // Display hull and energy
  int hullPercentage = (PlayerShipPtr->attributes.hullStrength * 100) /
                       PlayerShipPtr->shipType->baseHullStrength;
  printf("\nHull Integrity: %d%%", hullPercentage);
  printf("\nEnergy Banks: %.1f / %.1f", PlayerShipPtr->attributes.energyBanks,
         PlayerShipPtr->attributes.maxEnergyBanks);
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
  bool hasEquipment = false;
  // Check all equipment slots for active equipment
  for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
    if (PlayerShipPtr->equipment[i].isActive &&
        strlen(PlayerShipPtr->equipment[i].name) > 0 &&
        strcmp(PlayerShipPtr->equipment[i].name, "Empty") != 0) {

      hasEquipment = true;
      printf("\n  - %s", PlayerShipPtr->equipment[i].name);
    }
  }

  if (!hasEquipment) {
    printf("\n  No active equipment.");
  }

  printf("\n");
  return true;
}

static inline bool do_repair(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return false;
  }

  // Check if repair is needed
  if (PlayerShipPtr->attributes.hullStrength >=
      PlayerShipPtr->shipType->baseHullStrength) {
    printf("\nYour ship doesn't need any repairs.");
    return true;
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
    return false;
  }

  // Perform the repair
  Cash -= repairCost * 10; // Convert to internal units
  PlayerShipPtr->attributes.hullStrength =
      PlayerShipPtr->shipType->baseHullStrength;

  printf("\nShip repaired for %.1f credits. Hull integrity restored to 100%%.",
         (float)repairCost);
  return true;
}

static inline bool do_ship_details(char *commandArguments) {
  (void)(commandArguments); // Mark commandArguments as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data is not available.");
    return false;
  }

  // Synchronize ship fuel with global state before displaying details
  extern uint16_t Fuel;
  PlayerShipPtr->attributes.fuelLiters =
      Fuel * 10.0; // Convert game units to liters

  // Call the detailed ship status display function from elite_ship_types.h
  DisplayShipStatus(PlayerShipPtr);
  return true;
}

/**
 * Command to purchase and install equipment on the player's ship.
 * Usage: equip <equipment_name>
 * Available equipment depends on the current system's tech level.
 */
static inline bool do_purchase_equipment(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return false;
  }

  // Check if we are docked at a station
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION) {
    printf("\nYou must be docked at a station to purchase equipment.");
    return false;
  }

  // Check if an equipment name was provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: equip <equipment_name>");
    printf("\n\nAvailable Equipment:");
    printf("\n- ecm          - Electronic Counter Measures (600 CR)");
    printf("\n- fuelscoop    - Fuel Scoop (525 CR)");
    printf("\n- dockcomp     - Docking Computer (1500 CR)");
    printf("\n- escape       - Escape Pod (1000 CR)");
    printf("\n- energy       - Extra Energy Unit (1500 CR)");
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
    return true;
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
  double energyDraw = 0.0;
  double damageOutput = 0.0;

  // Match equipment name to available options
  if (strcmp(equipName, "ecm") == 0) {
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_ECM;
    slotType = EQUIPMENT_SLOT_TYPE_DEFENSIVE_1;
    formalName = "ECM System";
    cost = COST_ECM;
    requiredTechLevel = 2;
    energyDraw = 5.0;
  } else if (strcmp(equipName, "fuelscoop") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_FUEL_SCOOPS;
    slotType = UTILITY_SYSTEM_1;
    formalName = "Fuel Scoop";
    cost = COST_FUEL_SCOOPS;
    requiredTechLevel = 3;
    energyDraw = 3.0;
  } else if (strcmp(equipName, "dockcomp") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_DOCKING_COMPUTER;
    slotType = UTILITY_SYSTEM_2;
    formalName = "Docking Computer";
    cost = COST_DOCKING_COMPUTER;
    requiredTechLevel = 5;
    energyDraw = 2.0;
  } else if (strcmp(equipName, "escape") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_ESCAPE_POD;
    slotType = UTILITY_SYSTEM_3;
    formalName = "Escape Pod";
    cost = COST_ESCAPE_POD;
    requiredTechLevel = 5;
    energyDraw = 0.0;
  } else if (strcmp(equipName, "energy") == 0) {
    equipType.defensiveType = DEFENSIVE_SYSTEM_TYPE_EXTRA_ENERGY_UNIT;
    slotType = EQUIPMENT_SLOT_TYPE_DEFENSIVE_2;
    formalName = "Extra Energy Unit";
    cost = COST_EXTRA_ENERGY_UNIT;
    requiredTechLevel = 4;
    energyDraw = 0.0;
    // Apply the upgrade directly
    if (PurchaseEquipment(PlayerShipPtr, formalName, slotType, equipType, cost,
                          requiredTechLevel, energyDraw, damageOutput)) {
      // Also increase max energy banks
      PlayerShipPtr->attributes.maxEnergyBanks += EXTRA_ENERGY_UNIT_CAPACITY;
      return true;
    }
    return false;
  } else if (strcmp(equipName, "cargo") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_CARGO_BAY_EXTENSION;
    slotType = UTILITY_SYSTEM_4;
    formalName = "Cargo Bay Extension";
    cost = COST_CARGO_BAY_EXTENSION;
    requiredTechLevel = 1;
    energyDraw = 0.0;
    // Apply the cargo upgrade directly
    if (Cash < cost) {
      printf("\nInsufficient credits to purchase Cargo Bay Extension. "
             "Required: %d, Available: %.1f",
             cost, (float)Cash / 10.0f);
      return false;
    }

    if (techLevel < requiredTechLevel) {
      printf("\nCargo Bay Extensions not available at this tech level. "
             "Required: %d, Current: %d",
             requiredTechLevel + 1, techLevel + 1);
      return false;
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
    return true;
  } else if (strcmp(equipName, "pulse") == 0) {
    equipType.weaponType = WEAPON_TYPE_PULSE_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Pulse Laser";
    cost = COST_PULSE_LASER;
    requiredTechLevel = 1;
    energyDraw = 10.0;
    damageOutput = 5.0;
  } else if (strcmp(equipName, "beam") == 0) {
    equipType.weaponType = WEAPON_TYPE_BEAM_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Beam Laser";
    cost = COST_BEAM_LASER;
    requiredTechLevel = 3;
    energyDraw = 12.0;
    damageOutput = 7.5;
  } else if (strcmp(equipName, "military") == 0) {
    equipType.weaponType = WEAPON_TYPE_MILITARY_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Military Laser";
    cost = COST_MILITARY_LASER;
    requiredTechLevel = 6;
    energyDraw = 15.0;
    damageOutput = 10.0;
  } else if (strcmp(equipName, "mining") == 0) {
    equipType.weaponType = WEAPON_TYPE_MINING_LASER;
    slotType = EQUIPMENT_SLOT_TYPE_FORWARD_WEAPON;
    formalName = "Mining Laser";
    cost = COST_MINING_LASER;
    requiredTechLevel = 2;
    energyDraw = 12.0;
    damageOutput = 3.0;
  } else if (strcmp(equipName, "scanner") == 0) {
    equipType.utilityType = UTILITY_SYSTEM_TYPE_SCANNER_UPGRADE;
    slotType = UTILITY_SYSTEM_3;
    formalName = "Advanced Scanner";
    cost = COST_SCANNER_UPGRADE;
    requiredTechLevel = 4;
    energyDraw = 4.0;
  } else if (strcmp(equipName, "missile") == 0) {
    // For missiles, we just add to the count
    if (PlayerShipPtr->attributes.missilesLoadedHoming >=
        PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY) {
      printf("\nCannot purchase more missiles. All pylons are full.");
      return false;
    }

    if (Cash < COST_MISSILE_HOMING * 10) {
      printf("\nInsufficient credits to purchase missile. Required: %d, "
             "Available: %.1f",
             COST_MISSILE_HOMING, (float)Cash / 10.0f);
      return false;
    }

    Cash -= COST_MISSILE_HOMING * 10;
    PlayerShipPtr->attributes.missilesLoadedHoming++;
    printf("\nMissile purchased. Current missile count: %d/%d",
           PlayerShipPtr->attributes.missilesLoadedHoming,
           PlayerShipPtr->attributes.missilePylons * MISSILE_PYLON_CAPACITY);
    return true;
  } else {
    printf("\nUnknown equipment: %s", equipName);
    printf("\nUse 'equip' without parameters to see available equipment.");
    return false;
  }

  // Attempt to purchase the selected equipment
  bool result =
      PurchaseEquipment(PlayerShipPtr, formalName, slotType, equipType, cost,
                        requiredTechLevel, energyDraw, damageOutput);

  return result;
}

/**
 * Displays the equipment inventory of the player's ship.
 *
 * @param commandArguments Arguments provided to the command (unused)
 * @return true if the command was processed successfully
 */
static inline bool do_inventory_display(char *commandArguments) {
  (void)commandArguments; // Mark as unused

  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return false;
  }

  ListEquipmentInventory(PlayerShipPtr);
  return true;
}

/**
 * Stores equipment from a specified slot into the inventory.
 *
 * @param commandArguments Arguments provided to the command (slot number)
 * @return true if the equipment was stored successfully
 */
static inline bool do_store_equipment(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return false;
  }

  // Check if we're in combat
  if (InCombat) {
    printf("\nCannot modify ship configuration during combat.");
    return false;
  }

  // Check if a slot number was provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: store <slot_number>");
    printf("\n\nAvailable Equipment Slots:");
    PrintEquipmentSlots(PlayerShipPtr);
    return false;
  }

  // Parse the slot number
  int slotNumber = atoi(commandArguments);

  // Check if the slot number is valid
  if (slotNumber < 0 || slotNumber >= MAX_EQUIPMENT_SLOTS) {
    printf("\nInvalid slot number. Valid range: 0-%d", MAX_EQUIPMENT_SLOTS - 1);
    return false;
  }

  // Try to store the equipment
  return RemoveEquipmentToInventory(PlayerShipPtr, slotNumber);
}

/**
 * Equips an item from inventory into a specified slot.
 *
 * @param commandArguments Arguments provided to the command (inventory_index
 * slot_number)
 * @return true if the equipment was equipped successfully
 */
static inline bool do_equip_from_inventory(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return false;
  }

  // Check if we're in combat
  if (InCombat) {
    printf("\nCannot modify ship configuration during combat.");
    return false;
  }
  // Check if arguments were provided
  if (commandArguments == NULL || commandArguments[0] == '\0') {
    printf("\nUsage: use <inventory_index> <slot_number>\n");
    printf("Example: use 0 1  (equips item from inventory slot 0 to equipment "
           "slot 1)\n");
    printf("\nUse 'inv' command to view your inventory and 'shipinfo' to see "
           "available slots.\n");
    return false;
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
      return false;
    }
  } // Check if both arguments were provided and are valid
  if (invIndex < 0 || invIndex >= MAX_EQUIPMENT_INVENTORY) {
    printf("\nInvalid inventory index. Valid range: 0-%d\n",
           MAX_EQUIPMENT_INVENTORY - 1);
    return false;
  }

  if (slotNumber < 0 || slotNumber >= MAX_EQUIPMENT_SLOTS) {
    printf("\nInvalid slot number. Valid range: 0-%d\n",
           MAX_EQUIPMENT_SLOTS - 1);
    return false;
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
    return false;
  }

  // Get current system info
  extern char CurrentSystemName[20];      // From elite_player_state.h
  extern int CurrentSystemEconomy;        // From elite_player_state.h
  extern uint64_t currentGameTimeSeconds; // From elite_state.h
  extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

  // Display the shipyard
  DisplayShipyard(CurrentSystemName, CurrentSystemEconomy, PlayerShipPtr,
                  currentGameTimeSeconds);

  return true;
}

static inline bool do_compareship(char *args) {
  // Check if arguments are provided
  if (args == NULL || args[0] == '\0') {
    printf("Error: Please specify a ship to compare with.\n");
    printf("Usage: compareship <shipname>\n");
    return false;
  }

  // Get player ship
  extern PlayerShip *PlayerShipPtr; // From elite_player_state.h

  // Compare ships
  CompareShips(PlayerShipPtr, (const char *)args);

  return true;
}

static inline bool do_buyship(char *args) {
  // Check if player is docked at a station
  extern struct NavigationState PlayerNavState;
  extern int PlayerLocationType;

  // Need to be both at a station AND docked
  if (PlayerNavState.currentLocationType != CELESTIAL_STATION ||
      PlayerLocationType != 10) {
    printf("Error: You must be docked at a station to purchase a ship.\n");
    return false;
  }

  // Check if arguments are provided
  if (args == NULL || args[0] == '\0') {
    printf("Error: Please specify a ship to buy.\n");
    printf("Usage: buyship <ID or shipname> [notrade]\n");
    printf("Example: buyship 1  or  buyship \"Cobra Mk III\"\n");
    return false;
  }

  // Get current system info and player ship
  extern char CurrentSystemName[20];      // From elite_player_state.h
  extern int CurrentSystemEconomy;        // From elite_player_state.h
  extern uint64_t currentGameTimeSeconds; // From elite_state.h
  extern PlayerShip *PlayerShipPtr;       // From elite_player_state.h

  // Parse arguments
  char shipNameOrID[64] = {0};
  bool tradeIn = true;
  // Copy the first part of the arguments (up to the first space)
  const char *space = strchr(args, ' ');
  if (space != NULL) {
    size_t nameLen = space - args;
    nameLen = (nameLen < 63) ? nameLen : 63;
    snprintf(shipNameOrID, nameLen + 1, "%.*s", (int)nameLen, args);

    // Check for 'notrade' flag in the remaining part
    if (strstr(space + 1, "notrade") != NULL) {
      tradeIn = false;
    }
  } else {
    // No space, just copy the entire argument
    snprintf(shipNameOrID, sizeof(shipNameOrID), "%s", args);
  }

  // Check if the argument is a number (ID) or a string (ship name)
  char actualShipName[MAX_SHIP_NAME_LENGTH] = {0};
  bool isID = true;

  // Check if shipNameOrID is a number
  for (size_t i = 0; i < strlen(shipNameOrID); i++) {
    if (!isdigit(shipNameOrID[i])) {
      isID = false;
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
      return false;
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
  return true;
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
 * @return true if the cargo was successfully jettisoned
 */
static inline bool do_jettison(char *commandArguments) {
  if (PlayerShipPtr == NULL) {
    printf("\nError: Ship data not available.");
    return false;
  }

  if (commandArguments == NULL || strlen(commandArguments) == 0) {
    printf("\nUsage: jettison <cargo_name> <quantity>");
    printf("\nUsage: jettison all");
    printf("\nExample: jettison Food 5");
    return false;
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
    return false;
  }
  // No need to modify cargo name capitalization since we use case-insensitive
  // comparison The StringCompareIgnoreCase function will handle different
  // capitalizations

  // Find the cargo index in the ShipHold array (needed for synchronization)
  uint16_t cargoIndex = 0;
  bool cargoFound = false;

  // First, check if the cargo exists in the player's ship
  if (!GetCargoQuantity(PlayerShipPtr, cargoName)) {
    printf("\nError: %s not found in cargo hold.", cargoName);
    return false;
  }

  // Find the cargo index in the global tradnames array
  for (uint16_t i = 0; i <= LAST_TRADE; i++) {
    if (StringCompareIgnoreCase(tradnames[i], cargoName) == 0) {
      cargoIndex = i;
      cargoFound = true;
      break;
    }
  }

  if (!cargoFound) {
    printf("\nError: Unable to find cargo in global inventory. Please report "
           "this bug.");
    return false;
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
      return true;
    } else {
      printf(
          "\nError: Global cargo quantity mismatch. Please report this bug.");
      // Try to recover by synchronizing
      SynchronizeCargoSystems(PlayerShipPtr);
      return false;
    }
  } else {
    printf("\nFailed to jettison %s. Check cargo name and quantity.",
           cargoName);
    return false;
  }
}

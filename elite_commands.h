#ifndef ELITE_COMMANDS_H
#define ELITE_COMMANDS_H

// Include headers for necessary typedefs and forward declarations
#include "elite_navigation_types.h" // For CelestialType and NavigationState
#include "elite_star_system.h"		// For Star, Planet, Station, etc.

// Include the rest of the headers
#include "elite_state.h"		// Unified header for constants, structures, and globals
#include "elite_navigation.h"	// For distance, find_matching_system_name, execute_jump_to_planet
#include "elite_planet_info.h"	// For print_system_info (and goat_soup)
#include "elite_market.h"		// For execute_buy_order, execute_sell_order, display_market_info
#include "elite_player_state.h" // For calculate_fuel_purchase
#include "elite_save.h"			// For save_game, load_game
#include <stdlib.h>				// For atoi, atof
#include <math.h>				// For floor, fabs
#include <string.h>				// For string operations
#include <time.h>				// For time functions
#include <windows.h>			// For Windows directory operations

static inline bool do_tweak_random_native(char *commandArguments)
{
	(void)commandArguments; // Mark 's' as unused
	NativeRand ^= 1;
	return true;
}

static inline bool do_local_systems_display(char *commandArguments)
{
	uint16_t d = (uint16_t)atoi(commandArguments); // commandArguments might be unused if not parsed for a distance limit

	printf("Galaxy number %i", GalaxyNum);
	for (PlanetNum syscount = 0; syscount < GAL_SIZE; ++syscount)
	{
		d = distance(Galaxy[syscount], Galaxy[CurrentPlanet]);

		if (d <= MaxFuel)
		{
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

static inline bool do_jump(char *commandArguments)
{
	uint16_t d;
	PlanetNum dest = find_matching_system_name(commandArguments);

	if (dest == CurrentPlanet)
	{
		printf("\nBad jump");
		return false;
	}

	d = distance(Galaxy[dest], Galaxy[CurrentPlanet]);

	if (d > Fuel)
	{
		printf("\nJump too far");
		return false;
	}

	Fuel -= d;
	execute_jump_to_planet(dest);
	print_system_info(Galaxy[CurrentPlanet], false);
	return true;
}

static inline bool do_sneak(char *commandArguments)
{
	uint16_t fuelkeep = Fuel;
	bool b;
	Fuel = 666; // Arbitrary large fuel value for sneak
	b = do_jump(commandArguments);
	Fuel = fuelkeep;
	return b;
}

static inline bool do_galactic_hyperspace(char *commandArguments)
{
	(void)(commandArguments); /* Discard s */
	GalaxyNum++;
	if (GalaxyNum == 9)
	{
		GalaxyNum = 1;
	}

	// Create galaxy seed based on GalaxyNum
	struct SeedType galaxySeed;
	galaxySeed.a = BASE_0; // 0x5A4A
	galaxySeed.b = BASE_1; // 0x0248
	galaxySeed.c = BASE_2; // 0xB753
	galaxySeed.d = BASE_2; // Match original seed for Galaxy 1

	// For galaxies beyond 1, apply tweak_seed the appropriate number of times
	for (uint16_t i = 1; i < GalaxyNum; i++)
	{
		next_galaxy(&galaxySeed);
	}

	build_galaxy_data(galaxySeed);

	// Consider printing some confirmation or new system info
	printf("\nJumped to Galaxy %d. Current system: %s\n", GalaxyNum, Galaxy[CurrentPlanet].name);
	print_system_info(Galaxy[CurrentPlanet], false);
	return true;
}

static inline bool do_planet_info_display(char *commandArguments)
{
	PlanetNum dest = find_matching_system_name(commandArguments);
	if (dest < GAL_SIZE)
	{ // Check if a valid planet was found
		print_system_info(Galaxy[dest], false);
	}
	else
	{
		printf("\nPlanet not found: %s", commandArguments);
		return false;
	}
	return true;
}

static inline bool do_hold(char *commandArguments)
{
	uint16_t a = (uint16_t)atoi(commandArguments);
	uint16_t t = 0;

	for (uint16_t i = 0; i <= LAST_TRADE; ++i)
	{
		if ((Commodities[i].units) == TONNES_UNIT)
			t += ShipHold[i];
	}

	if (a < t) // Can't set hold space to less than current cargo
	{
		printf("\nHold too full to reduce size to %u. Current cargo: %u tonnes.", a, t);
		return false;
	}

	// Max hold space could be a constant or configurable
	// For now, let's assume a reasonable upper limit if needed, or just use what user provides
	// if (a > MAX_POSSIBLE_HOLD_SPACE) {
	//     printf("\nMaximum hold space is %u.", MAX_POSSIBLE_HOLD_SPACE);
	//     return false;
	// }

	HoldSpace = a - t;
	printf("\nHold space set to %u. Available: %u tonnes.", a, HoldSpace);
	return true;
}

static inline bool do_sell(char *commandArguments)
{
	uint16_t i;
	uint16_t t;
	char s2[MAX_LEN];
	split_string_at_first_space(commandArguments, s2);
	uint16_t a = (uint16_t)atoi(commandArguments);

	if (a == 0)
		a = 1;

	i = match_string_in_array(s2, tradnames, LAST_TRADE + 1);

	if (i == 0)
	{
		printf("\nUnknown trade good: '%s'", s2);
		return false;
	}

	i -= 1; // Adjust index for 0-based array

	t = execute_sell_order(i, a);

	if (t == 0)
	{
		printf("Cannot sell any %s", tradnames[i]);
	}
	else
	{
		printf("\nSelling %i%s of %s", t, UnitNames[Commodities[i].units], tradnames[i]);
	}
	return true;
}

static inline bool do_buy(char *commandArguments)
{
	uint16_t i;
	uint16_t t;
	char s2[MAX_LEN];
	split_string_at_first_space(commandArguments, s2);
	uint16_t a = (uint16_t)atoi(commandArguments);

	if (a == 0)
		a = 1;

	i = match_string_in_array(s2, tradnames, LAST_TRADE + 1);

	if (i == 0)
	{
		printf("\nUnknown trade good: '%s'", s2);
		return false;
	}
	i -= 1; // Adjust index

	t = execute_buy_order(i, a);
	if (t == 0)
	{
		printf("Cannot buy any %s", tradnames[i]);
	}
	else
	{
		printf("\nBuying %i%s of %s", t, UnitNames[Commodities[i].units], tradnames[i]);
	}
	return true;
}

static inline bool do_fuel(char *commandArguments)
{
	if (commandArguments == NULL || commandArguments[0] == '\0')
	{
		printf("\nUsage: fuel <amount>");
		return false;
	}
	uint16_t f = calculate_fuel_purchase((uint16_t)floor(10 * atof(commandArguments)));
	if (f == 0)
	{
		printf("\nCan't buy any fuel");
	}
	else
	{
		// Deduct the cost from cash
		Cash -= f * FuelCost;
		// Add the fuel to the current fuel level, making sure not to exceed MaxFuel
		Fuel = (Fuel + f > MaxFuel) ? MaxFuel : Fuel + f;
		printf("\nBuying %.1fLY fuel", (float)f / 10.0f);
	}
	return true;
}

static inline bool do_cash(char *commandArguments)
{
	if (commandArguments == NULL || commandArguments[0] == '\0')
	{
		printf("\nUsage: cash <amount>");
		return false;
	}
	int a = (int)(10 * atof(commandArguments)); // Amount is in tenths of credits
	Cash += (long)a;

	if (a != 0)
	{
		printf("\nCash adjusted by %.1f. Current cash: %.1f CR.", (float)a / 10.0f, (float)Cash / 10.0f);
		return true;
	}

	printf("Number not understood for cash command.");
	return false;
}

static inline bool do_market_display(char *commandArguments)
{
	(void)commandArguments;

	// Display basic market information
	display_market_info(LocalMarket);

	// Display current location economic info if we're at a station
	if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
		PlayerNavState.currentLocation.station != NULL &&
		CurrentStarSystem != NULL && CurrentStarSystem->planSys != NULL)
	{

		Station *station = PlayerNavState.currentLocation.station;

		// Find parent planet for context
		Planet *parentPlanet = NULL;
		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !parentPlanet; i++)
		{
			Planet *planet = &CurrentStarSystem->planets[i];
			if (!planet)
				continue;

			for (uint8_t j = 0; j < planet->numStations; j++)
			{
				if (planet->stations[j] == station)
				{
					parentPlanet = planet;
					break;
				}
			}
		}

		// Display economy information
		printf("\n\n=== STATION ECONOMY ===");
		printf("\nSystem Economy: %s", EconNames[CurrentStarSystem->planSys->economy]);

		// Display specialization
		const char *specNames[] = {"Balanced", "Industrial", "Agricultural", "Mining"};
		if (station->specialization < 4)
		{
			printf("\nStation Specialization: %s", specNames[station->specialization]);
		}

		// Display market update time
		uint64_t timeSinceUpdate = game_time_get_seconds() - station->lastMarketUpdate;
		printf("\nLast Market Update: %llu seconds ago", (unsigned long long)timeSinceUpdate);

		// Add hint about best trades based on specialization
		printf("\n\nTrade Opportunities:");
		switch (station->specialization)
		{
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

static inline bool do_quit(char *commandArguments)
{
	(void)(commandArguments);
	printf("\nExiting Text Elite. Goodbye!\n");
	exit(ExitStatus);
	// This line will not be reached if ExitStatus leads to a successful exit.
	// It's here to satisfy the function signature if exit() somehow didn't terminate.
	return true;
}

static inline bool do_help(char *commandArguments)
{
	// If specific help is requested for a command, show detailed help
	if (commandArguments && strlen(commandArguments) > 0)
	{
		char command[MAX_LEN];
		strncpy(command, commandArguments, MAX_LEN - 1);
		command[MAX_LEN - 1] = '\0';

		// Convert to lowercase for case-insensitive matching
		for (char *p = command; *p; ++p)
			*p = tolower(*p);

		// Trading commands
		if (strcmp(command, "buy") == 0 || strcmp(command, "b") == 0)
		{
			printf("\nBUY <good> <amount> - Purchase goods from the market");
			printf("\n  <good>   - Type of trade good (e.g., Food, Computers)");
			printf("\n  <amount> - Quantity to buy (default: 1)");
			printf("\n  Example: buy Food 5");
			printf("\n  Note: You must be docked at a station with a market to buy goods.");
			return true;
		}

		if (strcmp(command, "sell") == 0 || strcmp(command, "s") == 0)
		{
			printf("\nSELL <good> <amount> - Sell goods to the market");
			printf("\n  <good>   - Type of trade good (e.g., Food, Computers)");
			printf("\n  <amount> - Quantity to sell (default: 1)");
			printf("\n  Example: sell Computers 3");
			printf("\n  Note: You must be docked at a station with a market to sell goods.");
			return true;
		}

		// Navigation commands
		if (strcmp(command, "jump") == 0 || strcmp(command, "j") == 0)
		{
			printf("\nJUMP <planetname> - Jump to another star system");
			printf("\n  <planetname> - Name of the destination system");
			printf("\n  Example: jump Lave");
			printf("\n  Note: Requires fuel equal to the distance in light years.");
			printf("\n        Use 'local' to see systems within jump range.");
			return true;
		}

		if (strcmp(command, "local") == 0 || strcmp(command, "l") == 0)
		{
			printf("\nLOCAL - List star systems within jump range");
			printf("\n  Systems marked with * are within current fuel range.");
			printf("\n  Systems marked with - are within maximum fuel capacity but require refueling.");
			printf("\n  Distances are shown in light years (LY).");
			return true;
		}

		if (strcmp(command, "galhyp") == 0 || strcmp(command, "g") == 0)
		{
			printf("\nGALHYP - Perform a galactic hyperspace jump");
			printf("\n  Jumps to the next galaxy (1-8).");
			printf("\n  No fuel is required for this special jump.");
			return true;
		}

		// Star system navigation commands
		if (strcmp(command, "system") == 0 || strcmp(command, "sys") == 0)
		{
			printf("\nSYSTEM - Display detailed information about the current star system");
			printf("\n  Shows information about the star, planets, stations, and your current location.");
			printf("\n  No parameters required.");
			return true;
		}

		if (strcmp(command, "travel") == 0 || strcmp(command, "t") == 0)
		{
			printf("\nTRAVEL [destination] - Travel within the current star system");
			printf("\n  Without parameters: Lists all available destinations.");
			printf("\n  [destination]: The location to travel to, using the numbering system:");
			printf("\n    0       - Travel to the central star");
			printf("\n    1-8     - Travel to a planet (number depends on system)");
			printf("\n    1.1-8.5 - Travel to a station (format: planet.station)");
			printf("\n    N       - Travel to the Nav Beacon");
			printf("\n  Example: travel 2    - Travel to the second planet");
			printf("\n  Example: travel 1.3  - Travel to the third station orbiting the first planet");
			printf("\n  Example: travel N    - Travel to the Nav Beacon");
			printf("\n  Note: Travel consumes game time based on distance.");
			return true;
		}

		if (strcmp(command, "scan") == 0)
		{
			printf("\nSCAN - Scan the current star system");
			printf("\n  Shows distances to all celestial bodies from your current location.");
			printf("\n  Also shows estimated travel times to each destination.");
			printf("\n  No parameters required.");
			return true;
		}

		if (strcmp(command, "dock") == 0 || strcmp(command, "d") == 0)
		{
			printf("\nDOCK - Dock with the current station");
			printf("\n  Must be at a station location before docking.");
			printf("\n  Use 'travel' to navigate to a station first.");
			printf("\n  Docking provides access to market and other station services.");
			printf("\n  No parameters required.");
			return true;
		}

		// Market commands
		if (strcmp(command, "mkt") == 0 || strcmp(command, "m") == 0)
		{
			printf("\nMKT - Display market information");
			printf("\n  Shows current market prices, cash, fuel level, and cargo status.");
			printf("\n  No parameters required.");
			printf("\n  Note: Market prices vary between systems based on economy type.");
			return true;
		}

		if (strcmp(command, "fuel") == 0 || strcmp(command, "f") == 0)
		{
			printf("\nFUEL <amount> - Purchase fuel");
			printf("\n  <amount> - Amount of fuel to buy in light years");
			printf("\n  Example: fuel 7");
			printf("\n  Note: Your maximum fuel capacity is 7 light years.");
			return true;
		}

		// Cargo and Money commands
		if (strcmp(command, "hold") == 0 || strcmp(command, "h") == 0)
		{
			printf("\nHOLD <amount> - Set cargo hold capacity");
			printf("\n  <amount> - Total cargo hold space in tonnes");
			printf("\n  Example: hold 20");
			printf("\n  Note: Cannot reduce hold space below current cargo volume.");
			return true;
		}

		if (strcmp(command, "cash") == 0 || strcmp(command, "c") == 0)
		{
			printf("\nCASH <+/-amount> - Adjust cash balance");
			printf("\n  <+/-amount> - Amount to add or subtract from cash balance");
			printf("\n  Example: cash +100.0  - Add 100 credits");
			printf("\n  Example: cash -50.5   - Subtract 50.5 credits");
			printf("\n  Note: This is a debug command for testing purposes.");
			return true;
		}

		// Game management commands
		if (strcmp(command, "save") == 0)
		{
			printf("\nSAVE [description] - Save the current game state");
			printf("\n  [description] - Optional description of the save (e.g., 'At Lave')");
			printf("\n  Example: save Trading at Lave");
			printf("\n  Note: Save files are timestamped and stored in the game directory.");
			return true;
		}

		if (strcmp(command, "load") == 0)
		{
			printf("\nLOAD - List and load saved games");
			printf("\n  Shows a list of available save files, sorted by most recent first.");
			printf("\n  Enter the number of the save file to load when prompted.");
			printf("\n  Note: Loading a save will discard your current game state.");
			return true;
		}

		if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0)
		{
			printf("\nQUIT - Exit the game");
			printf("\n  Exits the game without saving. Use 'save' first to preserve your progress.");
			return true;
		}

		// Debug commands
		if (strcmp(command, "rand") == 0)
		{
			printf("\nRAND - Toggle random number generator");
			printf("\n  Switches between native and portable RNG implementations.");
			printf("\n  This is a debug command for testing purposes.");
			return true;
		}

		if (strcmp(command, "sneak") == 0)
		{
			printf("\nSNEAK <planetname> - Jump to another system without using fuel");
			printf("\n  <planetname> - Name of the destination system");
			printf("\n  Example: sneak Lave");
			printf("\n  Note: This is a debug command for testing purposes.");
			return true;
		}

		if (strcmp(command, "info") == 0 || strcmp(command, "i") == 0)
		{
			printf("\nINFO <planetname> - Display information about a system");
			printf("\n  <planetname> - Name of the system to get information about");
			printf("\n  Example: info Lave");
			printf("\n  Shows economy, government, tech level, and other system details.");
			return true;
		}

		if (strcmp(command, "compare") == 0)
		{
			printf("\nCOMPARE - Compare markets across different stations in the system");
			printf("\n  Shows price differences and profit opportunities between stations.");
			printf("\n  Lists all stations in the system with their distance from you.");
			printf("\n  Highlights best commodities to buy or sell at each station.");
			printf("\n  Shows estimated travel times to other stations.");
			printf("\n  Note: You must be docked at a station to use this command.");
			return true;
		}

		if (strcmp(command, "mkt") == 0 || strcmp(command, "m") == 0)
		{
			printf("\nMKT - Display market information");
			printf("\n  Shows current market prices, cash, fuel level, and cargo status.");
			printf("\n  No parameters required.");
			printf("\n  Note: Market prices vary between systems based on economy type.");
			return true;
		}

		if (strcmp(command, "fuel") == 0 || strcmp(command, "f") == 0)
		{
			printf("\nFUEL <amount> - Purchase fuel");
			printf("\n  <amount> - Amount of fuel to buy in light years");
			printf("\n  Example: fuel 7");
			printf("\n  Note: Your maximum fuel capacity is 7 light years.");
			return true;
		}

		// Cargo and Money commands
		if (strcmp(command, "hold") == 0 || strcmp(command, "h") == 0)
		{
			printf("\nHOLD <amount> - Set cargo hold capacity");
			printf("\n  <amount> - Total cargo hold space in tonnes");
			printf("\n  Example: hold 20");
			printf("\n  Note: Cannot reduce hold space below current cargo volume.");
			return true;
		}

		if (strcmp(command, "cash") == 0 || strcmp(command, "c") == 0)
		{
			printf("\nCASH <+/-amount> - Adjust cash balance");
			printf("\n  <+/-amount> - Amount to add or subtract from cash balance");
			printf("\n  Example: cash +100.0  - Add 100 credits");
			printf("\n  Example: cash -50.5   - Subtract 50.5 credits");
			printf("\n  Note: This is a debug command for testing purposes.");
			return true;
		}

		// Game management commands
		if (strcmp(command, "save") == 0)
		{
			printf("\nSAVE [description] - Save the current game state");
			printf("\n  [description] - Optional description of the save (e.g., 'At Lave')");
			printf("\n  Example: save Trading at Lave");
			printf("\n  Note: Save files are timestamped and stored in the game directory.");
			return true;
		}

		if (strcmp(command, "load") == 0)
		{
			printf("\nLOAD - List and load saved games");
			printf("\n  Shows a list of available save files, sorted by most recent first.");
			printf("\n  Enter the number of the save file to load when prompted.");
			printf("\n  Note: Loading a save will discard your current game state.");
			return true;
		}

		if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0)
		{
			printf("\nQUIT - Exit the game");
			printf("\n  Exits the game without saving. Use 'save' first to preserve your progress.");
			return true;
		}

		// Debug commands
		if (strcmp(command, "rand") == 0)
		{
			printf("\nRAND - Toggle random number generator");
			printf("\n  Switches between native and portable RNG implementations.");
			printf("\n  This is a debug command for testing purposes.");
			return true;
		}

		if (strcmp(command, "sneak") == 0)
		{
			printf("\nSNEAK <planetname> - Jump to another system without using fuel");
			printf("\n  <planetname> - Name of the destination system");
			printf("\n  Example: sneak Lave");
			printf("\n  Note: This is a debug command for testing purposes.");
			return true;
		}

		if (strcmp(command, "info") == 0 || strcmp(command, "i") == 0)
		{
			printf("\nINFO <planetname> - Display information about a system");
			printf("\n  <planetname> - Name of the system to get information about");
			printf("\n  Example: info Lave");
			printf("\n  Shows economy, government, tech level, and other system details.");
			return true;
		}

		if (strcmp(command, "compare") == 0)
		{
			printf("\nCOMPARE - Compare markets across different stations in the system");
			printf("\n  Shows price differences and profit opportunities between stations.");
			printf("\n  Lists all stations in the system with their distance from you.");
			printf("\n  Highlights best commodities to buy or sell at each station.");
			printf("\n  Shows estimated travel times to other stations.");
			printf("\n  Note: You must be docked at a station to use this command.");
			return true;
		}

		// If command not recognized, show general help
		printf("\nUnknown command: %s", command);
		printf("\nUse 'help' without parameters to see all available commands.");
		return true;
	}

	// Display general help categories
	printf("\n=== TXTELITE COMMAND REFERENCE ===");
	printf("\n\nTRADING COMMANDS:");
	printf("\n  buy   <good> <amount>   - Buy goods");
	printf("\n  sell  <good> <amount>   - Sell goods");
	printf("\n  mkt                     - Show current market prices, fuel, and cash");
	printf("\n  compare                 - Compare markets across stations in the system");

	printf("\n\nINTERSTELLAR NAVIGATION:");
	printf("\n  jump  <planetname>      - Jump to planet (uses fuel)");
	printf("\n  fuel  <amount>          - Buy amount Light Years of fuel");
	printf("\n  galhyp                  - Jump to the next galaxy");
	printf("\n  local                   - List systems within 7 light years");
	printf("\n  info  <planetname>      - Display information about a system");

	printf("\n\nSTAR SYSTEM NAVIGATION:");
	printf("\n  system                  - Display detailed information about the current star system");
	printf("\n  travel [destination]    - List destinations or travel within the system");
	printf("\n  scan                    - Scan system for points of interest and travel times");
	printf("\n  dock                    - Dock with a station if at a station location");

	printf("\n\nCARGO AND MONEY:");
	printf("\n  hold  <amount>          - Set total cargo hold space in tonnes");
	printf("\n  cash  <+/-amount>       - Adjust cash (e.g., cash +100.0 or cash -50.5)");

	printf("\n\nGAME MANAGEMENT:");
	printf("\n  save  [description]     - Save the game with optional description");
	printf("\n  load  [filename]        - List save games or load a specific save");
	printf("\n  quit                    - Exit the game");

	printf("\n\nDEBUG COMMANDS:");
	printf("\n  sneak <planetname>      - Jump to planet (no fuel cost, debug)");
	printf("\n  rand                    - Toggle RNG between native and portable (debug)");

	printf("\n\nFor detailed help on any command, type 'help <command>'");
	printf("\nAbbreviations allowed for most commands (e.g., b fo 5 for Buy Food 5, m for mkt).\n");
	return true;
}

static inline bool do_save(char *commandArguments)
{
	// Generate filename with current date and time
	char filename[64];
	time_t now = time(NULL);
	struct tm *timeinfo = localtime(&now);
	strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.sav", timeinfo);

	// Use the provided description if available
	char *description = NULL;

	// If command arguments were provided, use them as description
	if (commandArguments && commandArguments[0] != '\0')
	{
		description = commandArguments;
	}

	// Save the game
	bool success = save_game(filename, description);

	return success;
}

static inline bool do_load(char *commandArguments)
{
	(void)commandArguments; // Unused parameter

	printf("\nAvailable save files:\n");

	// Structure to store save file information
	typedef struct
	{
		char filename[MAX_PATH];
		time_t timestamp;
	} SaveFileInfo;

	// Find all .sav files
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile("*.sav", &findData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("No save files found.\n");
		return false;
	}

	// Count the number of save files
	int fileCount = 0;
	SaveFileInfo saveFiles[100]; // Array to store up to 100 save files

	do
	{
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// Store filename
			strncpy(saveFiles[fileCount].filename, findData.cFileName, MAX_PATH - 1);
			saveFiles[fileCount].filename[MAX_PATH - 1] = '\0';

			// Get file timestamp
			FILETIME ftCreate, ftAccess, ftWrite;
			SYSTEMTIME stUTC, stLocal;

			HANDLE hFile = CreateFile(findData.cFileName,
									  GENERIC_READ,
									  FILE_SHARE_READ,
									  NULL,
									  OPEN_EXISTING,
									  FILE_ATTRIBUTE_NORMAL,
									  NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				if (GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
				{
					FileTimeToSystemTime(&ftWrite, &stUTC);
					SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

					// Convert to time_t for sorting
					struct tm tm;
					memset(&tm, 0, sizeof(struct tm));
					tm.tm_year = stLocal.wYear - 1900;
					tm.tm_mon = stLocal.wMonth - 1;
					tm.tm_mday = stLocal.wDay;
					tm.tm_hour = stLocal.wHour;
					tm.tm_min = stLocal.wMinute;
					tm.tm_sec = stLocal.wSecond;
					tm.tm_min = stLocal.wMinute;
					tm.tm_sec = stLocal.wSecond;
					saveFiles[fileCount].timestamp = mktime(&tm);
				}
				CloseHandle(hFile);
			}

			fileCount++;
			if (fileCount >= 100)
				break; // Limit to 100 files
		}
	} while (FindNextFile(hFind, &findData) != 0);

	FindClose(hFind);

	if (fileCount == 0)
	{
		printf("No save files found.\n");
		return false;
	}

	// Sort save files by timestamp (most recent first)
	for (int i = 0; i < fileCount - 1; i++)
	{
		for (int j = 0; j < fileCount - i - 1; j++)
		{
			if (saveFiles[j].timestamp < saveFiles[j + 1].timestamp)
			{
				// Swap
				SaveFileInfo temp = saveFiles[j];
				saveFiles[j] = saveFiles[j + 1];
				saveFiles[j + 1] = temp;
			}
		}
	}

	// Display the sorted save files
	for (int i = 0; i < fileCount; i++)
	{
		// Read save header to get the description
		SaveHeader header;
		FILE *file = fopen(saveFiles[i].filename, "rb");
		bool headerValid = false;

		if (file)
		{
			if (fread(&header, sizeof(header), 1, file) == 1)
			{
				headerValid = (strncmp(header.signature, SAVE_SIGNATURE, strlen(SAVE_SIGNATURE)) == 0);
			}
			fclose(file);
		}

		// Format date and time
		struct tm *timeinfo = localtime(&saveFiles[i].timestamp);
		char timeStr[32];
		strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

		printf("%2d. %s - %s", i + 1, saveFiles[i].filename, timeStr);
		if (headerValid)
		{
			printf(" - %s", header.description);
		}
		printf("\n");
	}

	// Prompt user to select a save file
	if (fileCount > 0)
	{
		printf("\nEnter the number of the save file to load (or 0 to cancel): ");
		char input[10];
		if (fgets(input, sizeof(input), stdin) != NULL)
		{
			int selection = atoi(input);
			if (selection > 0 && selection <= fileCount)
			{
				return load_game(saveFiles[selection - 1].filename);
			}
		}
	}

	return false;
}

// =============================
// Star System Commands
// =============================

// Displays detailed information about the current star system
static inline bool do_system_info(char *commandArguments)
{
	(void)(commandArguments); // Unused parameter

	// Validate star system data
	if (!CurrentStarSystem)
	{
		printf("\nError: Star system data not available. System might not be properly initialized.");
		return false;
	}

	// Validate pointer to PlanSys data
	if (!CurrentStarSystem->planSys)
	{
		printf("\nError: Planet system data not available.");
		return false;
	}

	// Get current location information
	char locBuffer[MAX_LEN];
	get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

	// System header with basic information
	printf("\n===================================================");
	printf("\n             STAR SYSTEM: %s", CurrentStarSystem->planSys->name);
	printf("\n===================================================");

	// Economic and political information
	printf("\nEconomy: %s", EconNames[CurrentStarSystem->planSys->economy]);
	printf("\nGovernment: %s", GovNames[CurrentStarSystem->planSys->govType]);
	printf("\nTech Level: %d", CurrentStarSystem->planSys->techLev + 1);
	printf("\nPopulation: %u Billion", (CurrentStarSystem->planSys->population) >> 3);

	// Star information with spectral classification
	const char *spectralClasses[] = {"O", "B", "A", "F", "G", "K", "M"};
	printf("\n\nStar: %s", CurrentStarSystem->centralStar.name);
	if (CurrentStarSystem->centralStar.spectralClass < 7)
	{
		printf("\n  Class: %s (%.1f solar masses, %.1f luminosity)",
			   spectralClasses[CurrentStarSystem->centralStar.spectralClass],
			   CurrentStarSystem->centralStar.mass,
			   CurrentStarSystem->centralStar.luminosity);
	}

	// Planets information
	printf("\n\nPlanets: %d", CurrentStarSystem->numPlanets);
	if (CurrentStarSystem->numPlanets > 0)
	{
		// Planet type information for display
		const char *planetTypes[] = {
			"Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Giant"};

		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
		{
			Planet *planet = &CurrentStarSystem->planets[i];
			if (!planet)
			{
				printf("\n  %d. [Error: Invalid planet data]", i + 1);
				continue;
			}

			// Planet basic info
			printf("\n  %d. %s (%.2f AU)", i + 1, planet->name, planet->orbitalDistance);

			// Planet type and physical characteristics
			if (planet->type < 4)
			{
				printf("\n     Type: %s", planetTypes[planet->type]);
			}
			else
			{
				printf("\n     Type: Unknown");
			}
			printf("\n     Radius: %.0f km", planet->radius);

			// Station information for this planet
			if (planet->numStations > 0)
			{
				printf("\n     Stations: %d", planet->numStations);

				bool hasValidStations = false;
				for (uint8_t j = 0; j < planet->numStations; j++)
				{
					Station *station = planet->stations[j];
					if (!station)
						continue; // Skip NULL stations

					hasValidStations = true;
					// Station type information
					const char *stationTypes[] = {
						"Orbital", "Coriolis", "Ocellus"};

					printf("\n     %d.%d. %s (%.3f AU from planet)", i + 1, j + 1,
						   station->name, station->orbitalDistance);

					// Display station type if valid
					if (station->type < 3)
					{
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
						!station->hasMissions && !station->hasDockingComputer)
					{
						printf("None");
					}
				}

				if (!hasValidStations)
				{
					printf("\n     [No valid stations data]");
				}
			}
			else
			{
				printf("\n     Stations: None");
			}
		}
	}
	else
	{
		printf("\n  (None)");
	}

	// Nav Beacon information
	printf("\n\nNav Beacon: %.2f AU from star", CurrentStarSystem->navBeaconDistance);

	// Current player location
	printf("\n\nCurrent location: %s (%.2f AU from star)", locBuffer, PlayerNavState.distanceFromStar);

	// System time
	char timeBuffer[MAX_LEN * 2];
	game_time_get_formatted(timeBuffer, sizeof(timeBuffer));
	printf("\n\nSystem Time: %s", timeBuffer);

	return true;
}

// Lists available destinations and allows travel within the system
static inline bool do_travel(char *commandArguments)
{
	// Check if star system data is properly initialized
	if (!CurrentStarSystem)
	{
		printf("\nError: Star system data not available. System might not be properly initialized.");
		return false;
	}

	// Get current location information for display
	char locBuffer[MAX_LEN];
	get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

	// If no arguments provided, just list destinations
	if (commandArguments == NULL || strlen(commandArguments) == 0 ||
		strspn(commandArguments, " \t\n\r") == strlen(commandArguments))
	{

		printf("\nCurrent location: %s (%.2f AU from star)", locBuffer, PlayerNavState.distanceFromStar);
		printf("\n\nAvailable destinations:");

		// Star
		printf("\n  0. %s (0.00 AU)", CurrentStarSystem->centralStar.name);

		// Planets and stations
		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
		{
			Planet *planet = &CurrentStarSystem->planets[i];
			if (!planet)
			{
				printf("\n  %d. [Error: Invalid planet data]", i + 1);
				continue;
			}
			printf("\n  %d. %s (%.2f AU)", i + 1, planet->name, planet->orbitalDistance);

			for (uint8_t j = 0; j < planet->numStations; j++)
			{
				Station *station = planet->stations[j];
				if (!station)
				{
					continue; // Skip invalid stations silently
				}
				printf("\n     %d.%d. %s (%.2f AU)", i + 1, j + 1, station->name,
					   planet->orbitalDistance + station->orbitalDistance);
			}
		}

		// Nav Beacon
		printf("\n  N. Nav Beacon (%.2f AU)", CurrentStarSystem->navBeaconDistance);

		printf("\n\nUse 'travel <destination number>' to travel (e.g., 'travel 1' or 'travel 1.2' or 'travel N')");
		return true;
	}

	// Parse destination string, trimming whitespace
	char destStr[MAX_LEN];
	strncpy(destStr, commandArguments, sizeof(destStr) - 1);
	destStr[sizeof(destStr) - 1] = '\0';

	// Trim leading and trailing whitespace
	char *start = destStr;
	char *end = destStr + strlen(destStr) - 1;

	while (*start && isspace((unsigned char)*start))
		start++;
	while (end > start && isspace((unsigned char)*end))
		*end-- = '\0';

	if (start != destStr)
	{
		memmove(destStr, start, strlen(start) + 1);
	}

	// If destination string is empty after trimming
	if (strlen(destStr) == 0)
	{
		printf("\nNo destination specified. Use 'travel' to see available destinations.");
		return false;
	}

	// Check for Nav Beacon special case
	if (destStr[0] == 'N' || destStr[0] == 'n')
	{
		// Check if already at Nav Beacon
		if (PlayerNavState.currentLocationType == CELESTIAL_NAV_BEACON)
		{
			printf("\nAlready at Nav Beacon.");
			return true;
		}

		printf("\nTravelling to Nav Beacon...");
		// Pass a dummy non-NULL pointer for consistency with the function signature
		void *dummy = &CurrentStarSystem; // Using any valid address as a dummy
		bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_NAV_BEACON, dummy);
		if (result)
		{
			printf("\nArrived at Nav Beacon (%.2f AU from star)", PlayerNavState.distanceFromStar);
			return true;
		}
		else
		{
			printf("\nFailed to travel to Nav Beacon.");
			return false;
		}
	}

	// Parse destination index(es) for planets and stations
	int primaryIndex = -1;
	int secondaryIndex = -1;

	// Check for format "1.2" (planet.station)
	char *dotPos = strchr(destStr, '.');
	if (dotPos)
	{
		*dotPos = '\0'; // Split string at the dot

		// Validate that we have valid digits
		for (char *p = destStr; *p; p++)
		{
			if (!isdigit((unsigned char)*p))
			{
				printf("\nInvalid planet number: %s. Must be a number.", destStr);
				return false;
			}
		}

		for (char *p = dotPos + 1; *p; p++)
		{
			if (!isdigit((unsigned char)*p))
			{
				printf("\nInvalid station number: %s. Must be a number.", dotPos + 1);
				return false;
			}
		}

		primaryIndex = atoi(destStr);
		secondaryIndex = atoi(dotPos + 1);

		// Validate index ranges
		if (primaryIndex <= 0)
		{
			printf("\nInvalid planet number: %d. Must be a positive number.", primaryIndex);
			return false;
		}

		if (secondaryIndex <= 0)
		{
			printf("\nInvalid station number: %d. Must be a positive number.", secondaryIndex);
			return false;
		}
	}
	else
	{
		// For just a planet or star, validate that we have valid digits or '0'
		if (strcmp(destStr, "0") == 0)
		{
			primaryIndex = 0;
		}
		else
		{
			for (char *p = destStr; *p; p++)
			{
				if (!isdigit((unsigned char)*p))
				{
					printf("\nInvalid destination number: %s. Must be a number or 'N' for Nav Beacon.", destStr);
					return false;
				}
			}
			primaryIndex = atoi(destStr);

			if (primaryIndex < 0)
			{
				printf("\nInvalid destination number: %d. Must be a non-negative number.", primaryIndex);
				return false;
			}
		}
	}

	// Special case for star (index 0)
	if (primaryIndex == 0)
	{
		// Check if already at star
		if (PlayerNavState.currentLocationType == CELESTIAL_STAR)
		{
			printf("\nAlready at %s.", CurrentStarSystem->centralStar.name);
			return true;
		}

		printf("\nTravelling to %s...", CurrentStarSystem->centralStar.name);
		bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_STAR,
										  &CurrentStarSystem->centralStar);
		if (result)
		{
			printf("\nArrived at %s (0.00 AU from star)", CurrentStarSystem->centralStar.name);
			return true;
		}
		else
		{
			printf("\nFailed to travel to %s.", CurrentStarSystem->centralStar.name);
			return false;
		}
	}

	// Adjust for 1-based indexing for planets
	primaryIndex--;

	// Check if planet index is valid
	if (primaryIndex < 0 || primaryIndex >= CurrentStarSystem->numPlanets)
	{
		printf("\nInvalid destination. Planet number %d does not exist in this system.", primaryIndex + 1);
		printf("\nThis system has %d planets. Use 'travel' to see available destinations.",
			   CurrentStarSystem->numPlanets);
		return false;
	}

	Planet *planet = &CurrentStarSystem->planets[primaryIndex];
	if (!planet)
	{
		printf("\nError: Invalid planet data for planet %d.", primaryIndex + 1);
		return false;
	}

	// If no secondary index, travel to planet
	if (secondaryIndex == -1)
	{
		// Check if already at this planet
		if (PlayerNavState.currentLocationType == CELESTIAL_PLANET &&
			PlayerNavState.currentLocation.planet == planet)
		{
			printf("\nAlready at %s.", planet->name);
			return true;
		}

		printf("\nTravelling to %s...", planet->name);
		bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_PLANET, planet);
		if (result)
		{
			printf("\nArrived at %s (%.2f AU from star)", planet->name, PlayerNavState.distanceFromStar);
			return true;
		}
		else
		{
			printf("\nFailed to travel to %s.", planet->name);
			return false;
		}
	}

	// Adjust for 1-based indexing for stations
	secondaryIndex--;

	// Check if station index is valid
	if (secondaryIndex < 0 || secondaryIndex >= planet->numStations)
	{
		printf("\nInvalid station. Planet %s has %d stations (numbered 1 to %d).",
			   planet->name, planet->numStations, planet->numStations);
		return false;
	}

	Station *station = planet->stations[secondaryIndex];
	if (!station)
	{
		printf("\nError: Station data not available for station %d of planet %s.",
			   secondaryIndex + 1, planet->name);
		return false;
	}

	// Check if already at this station
	if (PlayerNavState.currentLocationType == CELESTIAL_STATION &&
		PlayerNavState.currentLocation.station == station)
	{
		printf("\nAlready at %s.", station->name);
		return true;
	}

	printf("\nTravelling to %s...", station->name);
	bool result = travel_to_celestial(CurrentStarSystem, &PlayerNavState, CELESTIAL_STATION, station);
	if (result)
	{
		printf("\nArrived at %s (%.2f AU from star)", station->name, PlayerNavState.distanceFromStar);
		return true;
	}
	else
	{
		printf("\nFailed to travel to %s.", station->name);
		return false;
	}
}

// Scans the current system for points of interest
static inline bool do_scan(char *commandArguments)
{
	(void)(commandArguments); // Unused parameter

	// Check if star system data is available
	if (!CurrentStarSystem)
	{
		printf("\nError: Star system data not available. System might not be properly initialized.");
		return false;
	}

	char locBuffer[MAX_LEN];
	get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

	printf("\n==== SYSTEM SCAN: %s ====", CurrentStarSystem->planSys->name);
	printf("\nCurrent location: %s (%.2f AU from star)", locBuffer, PlayerNavState.distanceFromStar);

	// Display distances to all points of interest
	printf("\n\nDistances to points of interest:");

	// Star
	double distToStar = PlayerNavState.distanceFromStar;
	printf("\n  Star: %.2f AU", distToStar);

	// Planets and stations
	for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
	{
		Planet *planet = &CurrentStarSystem->planets[i];
		if (!planet)
		{
			printf("\n  Planet %d: [Error: Invalid planet data]", i + 1);
			continue;
		}

		double distToPlanet = fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);
		printf("\n  %s: %.2f AU", planet->name, distToPlanet);

		bool hasValidStations = false;
		for (uint8_t j = 0; j < planet->numStations; j++)
		{
			Station *station = planet->stations[j];
			if (!station)
				continue; // Skip invalid stations

			hasValidStations = true;
			double stationDist = planet->orbitalDistance + station->orbitalDistance;
			double distToStation = fabs(PlayerNavState.distanceFromStar - stationDist);
			printf("\n     %s: %.2f AU", station->name, distToStation);
		}

		if (planet->numStations > 0 && !hasValidStations)
		{
			printf("\n     [No valid stations]");
		}
	}

	// Nav Beacon
	double distToNavBeacon = fabs(PlayerNavState.distanceFromStar - CurrentStarSystem->navBeaconDistance);
	printf("\n  Nav Beacon: %.2f AU", distToNavBeacon);

	// Calculate travel times to each destination
	printf("\n\nEstimated travel times:");

	// Star
	uint32_t timeToStar = calculate_travel_time(PlayerNavState.distanceFromStar, 0.0);
	printf("\n  Star: %u minutes", timeToStar / 60);

	// Planets and stations
	for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
	{
		Planet *planet = &CurrentStarSystem->planets[i];
		if (!planet)
			continue;

		uint32_t timeToPlanet = calculate_travel_time(PlayerNavState.distanceFromStar, planet->orbitalDistance);
		printf("\n  %s: %u minutes", planet->name, timeToPlanet / 60);

		for (uint8_t j = 0; j < planet->numStations; j++)
		{
			Station *station = planet->stations[j];
			if (!station)
				continue; // Skip invalid stations

			double stationDist = planet->orbitalDistance + station->orbitalDistance;
			uint32_t timeToStation = calculate_travel_time(PlayerNavState.distanceFromStar, stationDist);
			printf("\n     %s: %u minutes", station->name, timeToStation / 60);
		}
	}

	// Nav Beacon
	uint32_t timeToNavBeacon = calculate_travel_time(PlayerNavState.distanceFromStar, CurrentStarSystem->navBeaconDistance);
	printf("\n  Nav Beacon: %u minutes", timeToNavBeacon / 60);

	// Display current game time
	char timeBuffer[MAX_LEN * 2];
	game_time_get_formatted(timeBuffer, sizeof(timeBuffer));
	printf("\n\nGame Time: %s", timeBuffer);

	// Small time cost for performing a scan (1 minute)
	game_time_advance(60);
	printf("\n\nScan complete. Elapsed time: 1 minute.");

	return true;
}

// Docks with a station if at a station location
static inline bool do_dock(char *commandArguments)
{
	(void)(commandArguments); // Unused parameter

	// Validate star system data
	if (!CurrentStarSystem)
	{
		printf("\nError: Star system data not available. System might not be properly initialized.");
		return false;
	}

	// Check the player's current location type
	if (PlayerNavState.currentLocationType != CELESTIAL_STATION)
	{
		// Provide a helpful message based on current location
		char locBuffer[MAX_LEN];
		get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

		printf("\nCannot dock: Not at a station. You are currently at %s.", locBuffer);
		printf("\nUse 'travel' to navigate to a station first.");

		// List nearby stations as a convenience
		bool stationsFound = false;
		printf("\n\nNearby stations:");

		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
		{
			Planet *planet = &CurrentStarSystem->planets[i];
			if (!planet)
				continue;

			for (uint8_t j = 0; j < planet->numStations; j++)
			{
				Station *station = planet->stations[j];
				if (!station)
					continue;

				double stationDist = planet->orbitalDistance + station->orbitalDistance;
				double distToStation = fabs(PlayerNavState.distanceFromStar - stationDist);

				// Show stations within 1 AU as "nearby"
				if (distToStation <= 1.0)
				{
					printf("\n  %s (%.2f AU away) - Use 'travel %d.%d' to reach",
						   station->name, distToStation, i + 1, j + 1);
					stationsFound = true;
				}
			}
		}

		if (!stationsFound)
		{
			printf("\n  No stations within 1 AU. Use 'scan' to find all stations in the system.");
		}

		return false;
	}

	// Validate station data
	Station *station = PlayerNavState.currentLocation.station;
	if (!station)
	{
		printf("\nError: Station data not available. Cannot complete docking procedure.");
		return false;
	}

	// Find the parent planet for better location context
	Planet *parentPlanet = NULL;
	for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !parentPlanet; i++)
	{
		Planet *planet = &CurrentStarSystem->planets[i];
		if (!planet)
			continue;

		for (uint8_t j = 0; j < planet->numStations; j++)
		{
			if (planet->stations[j] == station)
			{
				parentPlanet = planet;
				break;
			}
		}
	}
	// Docking procedure and feedback
	printf("\nDocking at %s...", station->name);

	// Small time delay for docking
	game_time_advance(60); // 1 minute

	printf("\nDocked successfully. Welcome to %s!", station->name);

	// If we have parent planet info, display it
	if (parentPlanet)
	{
		printf("\nLocation: Orbiting %s", parentPlanet->name);

		// Update and use this station's market if it has one
		if (station->hasMarket && CurrentStarSystem->planSys)
		{
			// Update the station's market to the current game time
			UpdateStationMarket(station, game_time_get_seconds(), parentPlanet, CurrentStarSystem->planSys);

			// Set the global market to this station's market
			UseStationMarket(station, parentPlanet, CurrentStarSystem->planSys);

			// Show economic specialization
			const char *specNames[] = {"Balanced", "Industrial", "Agricultural", "Mining"};
			if (station->specialization < 4)
			{
				printf("\nEconomic specialization: %s", specNames[station->specialization]);
			}
		}
	}

	// Display available services
	printf("\n\nAvailable services:");
	if (station->hasMarket)
	{
		printf("\n- Market (use 'mkt', 'buy', 'sell' commands)");
	}
	if (station->hasShipyard)
	{
		printf("\n- Shipyard (equipment upgrades available)");
	}
	if (station->hasMissions)
	{
		printf("\n- Mission Board (missions available)");
	}
	if (station->hasDockingComputer)
	{
		printf("\n- Docking Computer Installation");
	}

	// If no services are available
	if (!station->hasMarket && !station->hasShipyard &&
		!station->hasMissions && !station->hasDockingComputer)
	{
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
static inline bool do_land(char *commandArguments)
{
	(void)(commandArguments); // Unused parameter

	// Validate star system data
	if (!CurrentStarSystem)
	{
		printf("\nError: Star system data not available. System might not be properly initialized.");
		return false;
	}

	// Check the player's current location type
	if (PlayerNavState.currentLocationType != CELESTIAL_PLANET || !PlayerNavState.currentLocation.planet)
	{
		// Provide a helpful message based on current location
		char locBuffer[MAX_LEN];
		get_current_location_name(&PlayerNavState, locBuffer, sizeof(locBuffer));

		printf("\nCannot land: Not at a planet. You are currently at %s.", locBuffer);
		printf("\nUse 'travel' to navigate to a planet first.");

		// List nearby planets as a convenience
		bool planetsFound = false;
		printf("\n\nNearby planets:");

		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
		{
			Planet *planet = &CurrentStarSystem->planets[i];
			if (!planet)
				continue;

			double distToPlanet = fabs(PlayerNavState.distanceFromStar - planet->orbitalDistance);

			// Show planets within 1 AU as "nearby"
			if (distToPlanet <= 1.0)
			{
				printf("\n  %s (%.2f AU away) - Use 'travel %d' to reach",
					   planet->name, distToPlanet, i + 1);
				planetsFound = true;
			}
		}

		if (!planetsFound)
		{
			printf("\n  No planets within 1 AU. Use 'scan' to find all planets in the system.");
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
	if (CurrentStarSystem->planSys)
	{
		// Generate or update the planet's market using the correct functions
		if (!planet->planetaryMarket.isInitialized)
		{
			// Set market fluctuation for this planet
			planet->marketFluctuation = (CurrentStarSystem->planSys->goatSoupSeed.c + planet->type) % 16;
			planet->lastMarketUpdate = game_time_get_seconds();

			// Create a temporary station to use the market generation function
			Station tempStation;
			memset(&tempStation, 0, sizeof(Station));
			tempStation.marketFluctuation = planet->marketFluctuation;

			// Set specialization based on planet type
			if (planet->type <= 1)
			{									// Rocky or Terrestrial
				tempStation.specialization = 2; // Agricultural focus for terrestrial planets
			}
			else
			{
				tempStation.specialization = 3; // Mining focus for gas giants and ice planets
			}

			// Generate market and store in planet's market
			tempStation.market = GenerateStationMarket(&tempStation, planet, CurrentStarSystem->planSys);
			planet->planetaryMarket.market = tempStation.market;
			planet->planetaryMarket.isInitialized = true;
		}
		else
		{
			// Update existing market based on elapsed time
			uint64_t currentTime = game_time_get_seconds();

			// Only update if sufficient time has passed (at least 1 hour of game time)
			const uint64_t UPDATE_INTERVAL = 3600; // 1 hour in seconds

			if (currentTime - planet->lastMarketUpdate >= UPDATE_INTERVAL)
			{
				// Create temporary station for market update
				Station tempStation;
				memset(&tempStation, 0, sizeof(Station));
				tempStation.marketFluctuation = planet->marketFluctuation;
				tempStation.market = planet->planetaryMarket.market;
				tempStation.lastMarketUpdate = planet->lastMarketUpdate;

				// Set specialization based on planet type
				if (planet->type <= 1)
				{									// Rocky or Terrestrial
					tempStation.specialization = 2; // Agricultural focus
				}
				else
				{
					tempStation.specialization = 3; // Mining focus
				}

				// Update market using station market update function
				UpdateStationMarket(&tempStation, currentTime, planet, CurrentStarSystem->planSys);

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
		const char *planetTypes[] = {"Rocky/Airless", "Terrestrial", "Gas Giant", "Ice Planet"};
		if (planet->type < 4)
		{
			printf("\nPlanet Type: %s", planetTypes[planet->type]);
		}

		// Show economy information
		printf("\nSystem Economy: %s", EconNames[CurrentStarSystem->planSys->economy]);

		// Display resource specialization based on planet type
		const char *resourceTypes[] = {"Minerals", "Agriculture", "Gases", "Rare Elements"};
		printf("\nMain Resources: %s", resourceTypes[planet->type % 4]);

		// Display market update time
		uint64_t timeSinceUpdate = game_time_get_seconds() - planet->lastMarketUpdate;
		printf("\nLast Market Update: %llu seconds ago", (unsigned long long)timeSinceUpdate);

		printf("\n\nTrading post established. Use 'mkt' to view available goods.");
	}

	return true;
}

static inline void update_all_system_markets()
{
	// Check if star system data is properly initialized
	if (!CurrentStarSystem)
	{
		return;
	}

	// Get current game time
	uint64_t currentTime = game_time_get_seconds();

	// Update markets for all stations in the system
	for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
	{
		Planet *planet = &CurrentStarSystem->planets[i];
		if (!planet)
			continue;

		for (uint8_t j = 0; j < planet->numStations; j++)
		{
			Station *station = planet->stations[j];
			if (!station)
				continue;

			// Update this station's market
			UpdateStationMarket(station, currentTime, planet, CurrentStarSystem->planSys);
		}
	}
}

// Compare markets across different stations in the system or with the planet market
static inline bool do_compare_markets(char *commandArguments)
{
	(void)commandArguments;								   // Mark as unused
	if (!CurrentStarSystem || !CurrentStarSystem->planSys) // Added planSys check for safety
	{
		printf("\\nError: Star system data not available for market comparison.");
		return false;
	}

	MarketType baseMarketToCompare; // Changed MarketInfo to MarketType
	char baseLocationName[MAX_LEN];
	bool isPlanetBase = false; // Initialize isPlanetBase

	// Determine the base market for comparison
	if (PlayerNavState.currentLocationType == CELESTIAL_PLANET && PlayerNavState.currentLocation.planet)
	{
		Planet *currentPlanet = PlayerNavState.currentLocation.planet;
		if (!currentPlanet)
		{
			printf("\nError: Current planet data is invalid for comparison.");
			return false;
		}

		strncpy(baseLocationName, currentPlanet->name, MAX_LEN - 1);
		baseLocationName[MAX_LEN - 1] = '\0';
		isPlanetBase = true;

		// Ensure the planetary market is initialized and up-to-date.
		// UpdatePlanetaryMarket handles both initialization and updates.
		// Use local market data since we're at the planet
		baseMarketToCompare = LocalMarket;
	}
	else if (PlayerNavState.currentLocationType == CELESTIAL_STATION && PlayerNavState.currentLocation.station)
	{
		Station *currentStation = PlayerNavState.currentLocation.station;
		if (!currentStation)
		{
			printf("\nError: Current station data is invalid for comparison.");
			return false;
		}

		strncpy(baseLocationName, currentStation->name, MAX_LEN - 1);
		baseLocationName[MAX_LEN - 1] = '\0';
		isPlanetBase = false;

		Planet *orbitingPlanet = NULL;
		// Find the planet this station orbits for market context
		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; ++i)
		{
			Planet *p = &CurrentStarSystem->planets[i];
			if (!p)
				continue;
			for (uint8_t j = 0; j < p->numStations; ++j)
			{
				if (p->stations[j] == currentStation)
				{
					orbitingPlanet = p;
					break;
				}
			}
			if (orbitingPlanet)
				break;
		}

		if (orbitingPlanet)
		{
			// Ensure the station market is initialized and up-to-date.
			UpdateStationMarket(currentStation, game_time_get_seconds(), orbitingPlanet, CurrentStarSystem->planSys);
			// Use the local market (which should be set to this station's market)
			baseMarketToCompare = LocalMarket;
		}
		else
		{
			printf("\nError: Could not determine orbiting planet for station %s. Using potentially stale local market data.", currentStation->name);
			// Fallback to LocalMarket if orbiting planet not found.
			baseMarketToCompare = LocalMarket;
		}
	}
	else
	{
		printf("\nYou must be docked at a station or landed on a planet to compare markets.");
		return false;
	}

	printf("\n=== MARKET COMPARISON ===");
	printf("\nBase location: %s", baseLocationName);

	bool foundStationsToCompare = false;
	for (uint8_t i = 0; i < CurrentStarSystem->numPlanets; i++)
	{
		Planet *planet = &CurrentStarSystem->planets[i];
		if (!planet)
			continue;

		for (uint8_t j = 0; j < planet->numStations; j++)
		{
			Station *station = planet->stations[j];
			if (!station || !station->hasMarket)
				continue; // Skip stations without markets

			// Skip comparing base station to itself if the base is a station
			if (!isPlanetBase && PlayerNavState.currentLocation.station == station)
			{
				continue;
			}

			foundStationsToCompare = true;
			// Update the "other" station's market to current time to ensure fair comparison
			UpdateStationMarket(station, game_time_get_seconds(), planet, CurrentStarSystem->planSys);

			// Create a temporary market for comparison
			MarketType otherMarket;

			// Use the current station's market
			// This assumes UpdateStationMarket updates the station's market data directly
			UseStationMarket(station, planet, CurrentStarSystem->planSys);
			otherMarket = station->market;

			printf("\n\nStation: %s (Orbiting %s)", station->name, planet->name);
			printf("\n-----------------------------------");
			printf("\n%-12s %-8s %-8s %-8s %-8s", "Commodity", "Base", "Other", "Diff", "QtyDiff");

			for (uint16_t k = 0; k <= LAST_TRADE; k++)
			{
				// Skip invalid commodities
				if (Commodities[k].basePrice == 0)
					continue;

				double basePrice = baseMarketToCompare.price[k]; // Changed Price to price
				int baseQty = baseMarketToCompare.quantity[k];	 // Changed Quantity to quantity

				double otherPrice = otherMarket.price[k]; // Changed Price to price
				int otherQty = otherMarket.quantity[k];	  // Changed Quantity to quantity

				printf("\n%-12s %-8.1f %-8.1f %-8.1f %-8d",
					   tradnames[k],
					   (float)basePrice / 10.0f,
					   (float)otherPrice / 10.0f,
					   (float)(otherPrice - basePrice) / 10.0f,
					   otherQty - baseQty);
			}
		}
	}

	if (!foundStationsToCompare)
	{
		if (isPlanetBase)
		{
			printf("\n\nNo other stations in the system with markets to compare against %s.", baseLocationName);
		}
		else
		{
			printf("\n\nNo other stations in the system with markets to compare against your current station %s.", baseLocationName);
			printf("\nOr you are at the only station with a market.");
		}
	}

	if (isPlanetBase)
	{
		printf("\n\nNote: Comparing all stations in the system to the planet market at %s.", baseLocationName);
	}
	else
	{
		printf("\n\nNote: Comparing all other stations in the system to your current station %s.", baseLocationName);
	}

	// Restore the original local market
	if (!isPlanetBase && PlayerNavState.currentLocation.station)
	{
		// Find planet for current station
		Planet *currentPlanet = NULL;
		Station *currentStation = PlayerNavState.currentLocation.station;

		for (uint8_t i = 0; i < CurrentStarSystem->numPlanets && !currentPlanet; i++)
		{
			Planet *p = &CurrentStarSystem->planets[i];
			if (!p)
				continue;

			for (uint8_t j = 0; j < p->numStations; j++)
			{
				if (p->stations[j] == currentStation)
				{
					currentPlanet = p;
					break;
				}
			}
		}

		if (currentPlanet)
		{
			UseStationMarket(currentStation, currentPlanet, CurrentStarSystem->planSys);
		}
	}

	return true;
}

#endif // ELITE_COMMANDS_H

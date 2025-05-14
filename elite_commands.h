#ifndef ELITE_COMMANDS_H
#define ELITE_COMMANDS_H

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_navigation.h" // For distance, find_matching_system_name, execute_jump_to_planet
#include "elite_planet_info.h" // For print_system_info (and goat_soup)
#include "elite_market.h" // For execute_buy_order, execute_sell_order, display_market_info
#include "elite_player_state.h" // For calculate_fuel_purchase
#include <stdlib.h> // For atoi, atof
#include <math.h> // For floor
#include <string.h> // For string operations
#include <math.h> // For floor

static inline bool do_tweak_random_native(char *commandArguments) 
{
	(void)commandArguments; // Mark 's' as unused
	NativeRand ^=1;
	return true;
}

static inline bool do_local_systems_display(char *commandArguments)
{
	uint16_t d = (uint16_t)atoi(commandArguments); // commandArguments might be unused if not parsed for a distance limit

	printf("Galaxy number %i", GalaxyNum);
	for(PlanetNum syscount = 0; syscount < GAL_SIZE; ++syscount)
	{
		d = distance(Galaxy[syscount], Galaxy[CurrentPlanet]);

		if(d <= MaxFuel)
		{
			if( d <= Fuel )
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

	if(dest == CurrentPlanet)
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
	(void)(commandArguments);     /* Discard s */
	GalaxyNum++;
	if(GalaxyNum == 9) { GalaxyNum = 1; }
	
	// Create galaxy seed based on GalaxyNum
	struct SeedType galaxySeed;
	galaxySeed.a = BASE_0; // 0x5A4A
	galaxySeed.b = BASE_1; // 0x0248
	galaxySeed.c = BASE_2; // 0xB753
	galaxySeed.d = BASE_2; // Match original seed for Galaxy 1
	
	// For galaxies beyond 1, apply tweak_seed the appropriate number of times
	for (uint16_t i = 1; i < GalaxyNum; i++) {
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
    if (dest < GAL_SIZE) { // Check if a valid planet was found
	    print_system_info(Galaxy[dest], false);
    } else {
        printf("\nPlanet not found: %s", commandArguments);
        return false;
    }
	return true;
}

static inline bool do_hold(char *commandArguments)
{
	uint16_t a = (uint16_t)atoi(commandArguments);
	uint16_t t = 0;

	for(uint16_t i = 0; i <= LAST_TRADE; ++i)
	{
		if ((Commodities[i].units) == TONNES_UNIT)
			t += ShipHold[i];
	}

	if(a < t) // Can't set hold space to less than current cargo
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

	if(i == 0)
	{
		printf("\nUnknown trade good: '%s'", s2);
		return false;
	} 

	i -= 1; // Adjust index for 0-based array

	t = execute_sell_order(i, a);

	if(t == 0)
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

	if (a == 0) a = 1;

	i = match_string_in_array(s2, tradnames, LAST_TRADE + 1);

	if(i == 0)
	{
		printf("\nUnknown trade good: '%s'", s2);
		return false;
	} 
	i -= 1; // Adjust index

	t = execute_buy_order(i, a);
	if(t == 0)
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
    if (commandArguments == NULL || commandArguments[0] == '\0') {
        printf("\nUsage: fuel <amount>");
        return false;
    }
	uint16_t f = calculate_fuel_purchase((uint16_t)floor(10 * atof(commandArguments)));
	if(f == 0) { 
        printf("\nCan't buy any fuel");
    }
	else { 
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
    if (commandArguments == NULL || commandArguments[0] == '\0') {
        printf("\nUsage: cash <amount>");
        return false;
    }
	int a = (int)(10 * atof(commandArguments)); // Amount is in tenths of credits
	Cash += (long)a;

	if(a != 0) {
        printf("\nCash adjusted by %.1f. Current cash: %.1f CR.", (float)a / 10.0f, (float)Cash / 10.0f);
		return true;
    }

	printf("Number not understood for cash command.");
	return false;
}

static inline bool do_market_display(char *commandArguments)
{
	(void)commandArguments; 
	display_market_info(LocalMarket); 

	printf("\nFuel :%.1fLY", (float)Fuel / 10.0f);
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
	(void)(commandArguments);
	printf("\nCommands are:");
	printf("\n  buy   <good> <amount>   - Buy goods");
	printf("\n  sell  <good> <amount>   - Sell goods");
	printf("\n  fuel  <amount>          - Buy amount Light Years of fuel");
	printf("\n  jump  <planetname>      - Jump to planet (uses fuel)");
	printf("\n  sneak <planetname>      - Jump to planet (no fuel cost, debug)");
	printf("\n  galhyp                  - Jump to the next galaxy");
	printf("\n  info  <planetname>      - Display information about a system");
	printf("\n  mkt                     - Show current market prices, fuel, and cash");
	printf("\n  local                   - List systems within 7 light years");
	printf("\n  cash  <+/-amount>       - Adjust cash (e.g., cash +100.0 or cash -50.5)");
	printf("\n  hold  <amount>          - Set total cargo hold space in tonnes");
	printf("\n  quit                    - Exit the game");
	printf("\n  rand                    - Toggle RNG between native and portable (debug)");
	printf("\n\nAbbreviations allowed (e.g., b fo 5 for Buy Food 5, m for mkt).\n");
	return true;
}

#endif // ELITE_COMMANDS_H

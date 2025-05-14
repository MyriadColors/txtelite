/* txtelite.c  1.5 */
/* Textual version of Elite trading (C implementation) */
/* Converted by Ian Bell from 6502 Elite sources.
   Original 6502 Elite by Ian Bell & David Braben. */


/* ----------------------------------------------------------------------
  The nature of basic mechanisms used to generate the Elite socio-economic
universe are now widely known. A competant games programmer should be able to
produce equivalent functionality. A competant hacker should be able to lift 
the exact system from the object code base of official conversions.

  This file may be regarded as defining the Classic Elite universe.

  It contains a C implementation of the precise 6502 algorithms used in the
 original BBC Micro version of Acornsoft Elite (apart from Galctic Hyperspace
 target systems) together with a parsed textual command testbed.

  Note that this is not the universe of David Braben's 'Frontier' series.


ICGB 13/10/99	; 21/07/15
ian@ianbell.me
www.ianbellelite.com
  ---------------------------------------------------------------------- */



/* Note that this program is "quick-hack" text parser-driven version
of Elite with no combat or missions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

// Forward declarations for structs
struct SeedType;
struct PlanSys;

#define MAX_LEN 30
#define GAL_SIZE (256)
#define ALIEN_ITEMS (16)
#define LAST_TRADE ALIEN_ITEMS
#define NUM_COMMANDS (14) // Renamed from nocomms

const int TONNES = 0;

int ExitStatus = EXIT_SUCCESS;

typedef int PlanetNum;

// Simplified struct definitions
struct FastSeedType {
    uint8_t a, b, c, d; // Renamed from A, B, C, D
}; // Four-byte random number used for planet description

struct SeedType {
    uint16_t w0; // Renamed from W0
    uint16_t w1; // Renamed from W1
    uint16_t w2; // Renamed from W2
}; // Six-byte random number used as seed for planets

struct PlanSys {
    uint16_t x; // Renamed from X
    uint16_t y; // Renamed from Y
    uint16_t economy; // Renamed from Economy
    uint16_t govType; // Renamed from GovType
    uint16_t techLev; // Renamed from TechLev
    uint16_t population;   // Renamed from Population
    uint16_t productivity; // Renamed from Productivity
    uint16_t radius;       // Renamed from Radius
    struct FastSeedType goatSoupSeed; // Renamed from GoatSoupSeed
    char name[12]; // Renamed from Name
};

static_assert(GAL_SIZE == 256, "Galaxy size must be 256");
static_assert(ALIEN_ITEMS == 16, "AlienItems must be 16");

#define NUM_FOR_LAVE 7       /* Lave is 7th generated planet in galaxy one */
#define NUM_FOR_ZAONCE 129
#define NUM_FOR_DISO 147
#define NUM_FOR_RIED 46

struct PlanSys Galaxy[GAL_SIZE]; /* Need 0 to galsize-1 inclusive */

struct SeedType Seed;

struct FastSeedType RndSeed;

bool NativeRand;

/* In 6502 version these were:
typedef struct
{
	uint8_t baseprice;
	uint8_t basequant;
	uint8_t maskbyte;
	int8_t gradient_units; sign/5bits/2bits
	char* name[20]; longest="Radioactives"
}
*/

typedef struct {
    uint16_t basePrice; // Renamed from BasePrice
    int16_t gradient; // Renamed from Gradient
    uint16_t baseQuant; // Renamed from BaseQuant
    uint16_t maskByte; // Renamed from MaskByte
    uint16_t units; // Renamed from Units
    char name[20]; // Renamed from Name
} TradeGood;

typedef struct {
    uint16_t quantity[LAST_TRADE + 1]; // Renamed from Quantity
    uint16_t price[LAST_TRADE + 1]; // Renamed from Price
} MarketType;

/* Player workspace */
uint16_t ShipHold[LAST_TRADE + 1];  /* Contents of cargo bay */
int CurrentPlanet;                 /* Current planet */
uint16_t GalaxyNum;                /* Galaxy number (1-8) */
int32_t Cash;
uint16_t Fuel;
MarketType LocalMarket;
uint16_t HoldSpace;

int FuelCost = 2; /* 0.2 CR/Light year */
int MaxFuel = 70; /* 7.0 LY tank */

const uint16_t BASE_0 = 0x5A4A;
const uint16_t BASE_1 = 0x0248;
const uint16_t BASE_2 = 0xB753;  /* Base seed for galaxy 1 */


//static const char *digrams=
//							 "ABOUSEITILETSTONLONUTHNO"
//							 "ALLEXEGEZACEBISO"
//							 "USESARMAINDIREA?"
//							 "ERATENBERALAVETI"
//							 "EDORQUANTEISRION";


#if 0 // 1.4-
char pairs0[]="ABOUSEITILETSTONLONUTHNO";
/* must continue into .. */
char pairs[] = "..LEXEGEZACEBISO"
"USESARMAINDIREA."
"ERATENBERALAVETI"
"EDORQUANTEISRION"; /* Dots should be nullprint characters */


#else // 1.5 planet names fix
char pairs0[]=
"ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSESARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

char pairs[] = "..LEXEGEZACEBISO"
"USESARMAINDIREA."
"ERATENBERALAVETI"
"EDORQUANTEISRION"; /* Dots should be nullprint characters */

#endif

char GovNames[][MAX_LEN] = {"Anarchy", "Feudal", "Multi-gov", "Dictatorship",
	"Communist", "Confederacy", "Democracy", "Corporate State"};

char EconNames[][MAX_LEN] = {"Rich Ind", "Average Ind", "Poor Ind", "Mainly Ind",
	"Mainly Agri", "Rich Agri", "Average Agri", "Poor Agri"};


char UnitNames[][5] ={"t","kg","g"};

/* Data for DB's price/availability generation system */
/* Base  Grad Base Mask Un   Name price ient quant     it              */ 

#define POLITICALLY_CORRECT	0
/* Set to 1 for NES-sanitised trade goods */

TradeGood Commodities[] = {
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

/* ================================ *
 * Required data for text interface *
 * ================================ */

/* Tradegood names used in text commands Set using commodities array */
char tradnames[LAST_TRADE][MAX_LEN]; 

// Forward function declarations
void tweak_seed(struct SeedType *seedToTweak);
struct PlanSys make_system(struct SeedType *initialSeed);
MarketType generate_market(uint16_t fluctuation, struct PlanSys planetSystem);
void next_galaxy(struct SeedType *currentSeed);
uint16_t distance(struct PlanSys systemA, struct PlanSys systemB);
void print_system_info(struct PlanSys planetSystemInfo, bool useCompressedOutput);
void goat_soup(const char *sourceString, struct PlanSys *planetSystem);

bool do_buy(char *commandArguments);
bool do_sell(char *commandArguments);
bool do_fuel(char *commandArguments);
bool do_jump(char *commandArguments);
bool do_cash(char *commandArguments);
bool do_market_display(char *commandArguments);
bool do_help(char *commandArguments);
bool do_hold(char *commandArguments);
bool do_sneak(char *commandArguments);
bool do_local_systems_display(char *commandArguments);
bool do_planet_info_display(char *commandArguments);
bool do_galactic_hyperspace(char *commandArguments);
bool do_quit(char *commandArguments);
bool do_tweak_random_native(char *commandArguments);

char commands[NUM_COMMANDS][MAX_LEN]=
{
	"buy",        "sell",     "fuel",     "jump",
	"cash",       "mkt",      "help",     "hold",
	"sneak",      "local",    "info",     "galhyp",
	"quit",       "rand"	
};

bool (*comfuncs[NUM_COMMANDS])(char *)=
{
	do_buy,         do_sell,       do_fuel,    do_jump,
	do_cash,        do_market_display,        do_help,    do_hold,
	do_sneak,       do_local_systems_display,      do_planet_info_display,    do_galactic_hyperspace,
	do_quit,                              do_tweak_random_native
};  

/* ================= *
 * General functions *
 * ================= */
void port_srand(unsigned int);
int port_rand(void);

static unsigned int lastrand = 0;

void my_srand(unsigned int initialSeed)
{
	srand(initialSeed);
	lastrand = initialSeed - 1;
}

int my_rand(void)
{
	int r;

	if(NativeRand) 
		r=rand();
	else
	{	// As supplied by D McDonnell	from SAS Insititute C
		r = (((((((((((lastrand << 3) - lastrand) << 3)
											+ lastrand) << 1) + lastrand) << 4)
							- lastrand) << 1) - lastrand) + 0xe60)
			& 0x7fffffff;
		lastrand = r - 1;	
	}
	return(r);
}

char random_byte(void)
{ 
	return (char)(my_rand()&0xFF);
}

uint16_t minimum_value(uint16_t valueA, uint16_t valueB)
{ 
	return valueA < valueB ? valueA : valueB;
}

void stop(char *messageString)
{
	printf("\n%s",messageString);
	exit(1);
}

signed int float_to_int_round(double inputValue)
{ 
	return ((signed int)floor(inputValue+0.5));
}

signed int float_to_int_floor(double inputValue)
{
	return ((signed int)floor(inputValue));
}

void tweak_seed(struct SeedType *seedToTweak)
{
	uint16_t temp;
	temp = ((*seedToTweak).w0)+((*seedToTweak).w1)+((*seedToTweak).w2); /* 2 byte aritmetic */
	(*seedToTweak).w0 = (*seedToTweak).w1;
	(*seedToTweak).w1 = (*seedToTweak).w2;
	(*seedToTweak).w2 = temp;
}

/* =================================== *
 * String functions for text interface *
 * =================================== */

/* Remove all c's from string s */
void strip_char_from_string(char *inputString, const char charToStrip)
{
	size_t i=0,j=0;

	while(i<strlen(inputString))
	{
		if(inputString[i]!=charToStrip) { inputString[j]=inputString[i]; j++;}
		i++;
	}

	inputString[j]=0;
}

/* Return nonzero iff string t begins with non-empty string s */
bool string_begins_with(char *prefixString, char *fullString)
{
	size_t i=0;
	size_t l=strlen(prefixString);
	if(l>0)
	{ while((i<l)&(toupper(prefixString[i])==toupper(fullString[i])))	i++;
		if(i==l) return true;
	}
	return false;
}

/* 
 * Check string s against n options in string array a
 * If matches ith element return i+1 else return 0
 */
uint16_t match_string_in_array(char *searchString, char stringArray[][MAX_LEN], uint16_t arraySize)
{
	for(uint16_t i = 0; i < arraySize; i++)
	{
		if( string_begins_with( searchString, stringArray[i] ) )
			return i + 1;
	}

	return 0;
}

/* Strip leading and trailing space characters from the given string. */
char *strip_leading_trailing_spaces(char *inputString)
{
	char *p;
	while (*inputString != '\0' && isspace(*inputString))
	{
		++inputString;
	}
	p = inputString + strlen(inputString);
	while (p > inputString && isspace(*(p - 1)))
	{
		--p;
		*p = '\0';
	}
	return inputString;
}

/* Split string s at first space, returning first 'word' in t & shortening s */
void split_string_at_first_space(char *inputString, char *firstWord)
{
	size_t l=strlen(inputString);
	size_t i=0,j=0;

	/* Strip leading spaces */
	for(; (i < l) &(inputString[i]==' '); i++);

	if(i == l)
	{
		inputString[0]=0;
		firstWord[0]=0;
		return;
	};

	for(; (i < l ) &(inputString[i] != ' '); i++, j++)
		firstWord[j] = inputString[i];

	firstWord[j]=0;
	i++;
	j=0;

	for(; i < l; i++, j++)
	{
		inputString[j] = inputString[i];
	}

	inputString[j]=0;
}

/* ========================== *
 * Functions for stock market *
 * ========================== */

/* 
 * Try to buy ammount a  of good i
 * Return ammount bought
 * Cannot buy more than is availble, can afford, or will fit in hold
 */
uint16_t execute_buy_order(uint16_t itemIndex, uint16_t amount)
{
	uint16_t t;
	if(Cash < 0) t=0;
	else
	{
		t=minimum_value(LocalMarket.quantity[itemIndex],amount);
		if ((Commodities[itemIndex].units)==TONNES) {t = minimum_value(HoldSpace,t);}
		t = minimum_value(t, (uint16_t)floor((double)Cash/(LocalMarket.price[itemIndex])));
	}
	ShipHold[itemIndex]+=t;
	LocalMarket.quantity[itemIndex]-=t;
	Cash-=t*(LocalMarket.price[itemIndex]);
	if ((Commodities[itemIndex].units)==TONNES)
		HoldSpace-=t;
	return t;
}

uint16_t execute_sell_order(uint16_t itemIndex, uint16_t amount) /* As gamebuy but selling */
{
	uint16_t t=minimum_value(ShipHold[itemIndex],amount);
	ShipHold[itemIndex]-=t;
	LocalMarket.quantity[itemIndex]+=t;
	if ((Commodities[itemIndex].units)==TONNES) {HoldSpace+=t;}
	Cash+=t*(LocalMarket.price[itemIndex]);
	return t;
}

MarketType generate_market(uint16_t fluctuation, struct PlanSys planetSystem)
{
	MarketType market;
	uint16_t i;
	for(i=0;i<=LAST_TRADE;i++)
	{
		int32_t q; 
		int32_t product = (planetSystem.economy)*(Commodities[i].gradient);
		int32_t changing = fluctuation & (Commodities[i].maskByte);
		q =  (Commodities[i].baseQuant) + changing - product;	
		q = q&0xFF;
		if(q&0x80) {q=0;};                       /* Clip to positive 8-bit */

		market.quantity[i] = (uint16_t)(q & 0x3F); /* Mask to 6 bits */

		q =  (Commodities[i].basePrice) + changing + product;
		q = q & 0xFF;
		market.price[i] = (uint16_t) (q*4);
	}
	market.quantity[ALIEN_ITEMS] = 0; /* Override to force nonavailability */
	return market;
}

void display_market_info(MarketType marketData)
{
	uint16_t i;
	for(i=0;i<=LAST_TRADE;i++)
	{ printf("\n");
		printf("%s", Commodities[i].name);
		printf("   %.1f",((float)(marketData.price[i])/10));
		printf("   %u",marketData.quantity[i]);
		printf("%s", UnitNames[Commodities[i].units]);
		printf("   %u",ShipHold[i]);
	}
}	


/* Generate system info from seed */
struct PlanSys make_system(struct SeedType *initialSeed)
{
	struct PlanSys thissys;
	uint16_t pair1,pair2,pair3,pair4;
	uint16_t longnameflag=((*initialSeed).w0)&64;

	thissys.x=(((*initialSeed).w1)>>8);
	thissys.y=(((*initialSeed).w0)>>8);

	thissys.govType =((((*initialSeed).w1)>>3)&7); /* bits 3,4 &5 of w1 */

	thissys.economy =((((*initialSeed).w0)>>8)&7); /* bits 8,9 &A of w0 */
	if (thissys.govType <=1)
	{ thissys.economy = ((thissys.economy)|2);
	} 

	thissys.techLev =((((*initialSeed).w1)>>8)&3)+((thissys.economy)^7);
	thissys.techLev +=((thissys.govType)>>1);
	if (((thissys.govType)&1)==1)	thissys.techLev+=1;
	/* C simulation of 6502's LSR then ADC */

	thissys.population = 4*(thissys.techLev) + (thissys.economy);
	thissys.population +=  (thissys.govType) + 1;

	thissys.productivity = (((thissys.economy)^7)+3)*((thissys.govType)+4);
	thissys.productivity *= (thissys.population)*8;

	thissys.radius = 256*(((((*initialSeed).w2)>>8)&15)+11) + thissys.x;  

	thissys.goatSoupSeed.a = (*initialSeed).w1 & 0xFF;;
	thissys.goatSoupSeed.b = (*initialSeed).w1 >>8;
	thissys.goatSoupSeed.c = (*initialSeed).w2 & 0xFF;
	thissys.goatSoupSeed.d = (*initialSeed).w2 >> 8;

	pair1=2*((((*initialSeed).w2)>>8)&31);  tweak_seed(initialSeed);
	pair2=2*((((*initialSeed).w2)>>8)&31);  tweak_seed(initialSeed);
	pair3=2*((((*initialSeed).w2)>>8)&31);  tweak_seed(initialSeed);
	pair4=2*((((*initialSeed).w2)>>8)&31);	tweak_seed(initialSeed);
	/* Always four iterations of random number */

	(thissys.name)[0]=pairs[pair1];
	(thissys.name)[1]=pairs[pair1+1];
	(thissys.name)[2]=pairs[pair2];
	(thissys.name)[3]=pairs[pair2+1];
	(thissys.name)[4]=pairs[pair3];
	(thissys.name)[5]=pairs[pair3+1];

	if(longnameflag) /* bit 6 of ORIGINAL w0 flags a four-pair name */
	{
		(thissys.name)[6]=pairs[pair4];
		(thissys.name)[7]=pairs[pair4+1];
		(thissys.name)[8]=0;
	}
	else (thissys.name)[6]=0;
	strip_char_from_string(thissys.name,'.');



	return thissys;
}  


/* =============== *
 * Generate galaxy *
 * =============== */

/* ================================= *
 * Functions for galactic hyperspace *
 * ================================= */

/* rotate 8 bit number leftwards */
/* (tried to use chars but too much effort persuading this braindead language to do bit operations on bytes!) */
uint16_t rotate_left(uint16_t valueToRotate) 
{
	uint16_t temp = valueToRotate&128;
	return (2*(valueToRotate&127))+(temp>>7);
} 

uint16_t twist(uint16_t valueToTwist)
{
	return (uint16_t)((256*rotate_left(valueToTwist>>8))+rotate_left(valueToTwist&255));
} 

/* Apply to base seed; once for galaxy 2  */
void next_galaxy(struct SeedType *currentSeed)
{ 
	(*currentSeed).w0 = twist((*currentSeed).w0);  /* twice for galaxy 3, etc. */
	(*currentSeed).w1 = twist((*currentSeed).w1);  /* Eighth application gives galaxy 1 again*/
	(*currentSeed).w2 = twist((*currentSeed).w2);
}

/* Original game generated from scratch each time info needed */
void build_galaxy_data(uint16_t galaxyNumber)
{
	uint16_t syscount,galcount;
	Seed.w0=BASE_0; Seed.w1=BASE_1; Seed.w2=BASE_2; /* Initialise seed for galaxy 1 */
	for(galcount=1;galcount<galaxyNumber;++galcount) next_galaxy(&Seed);
	/* Put galaxy data into array of structures */  
	for(syscount=0;syscount<GAL_SIZE;++syscount) Galaxy[syscount]=make_system(&Seed);
}

/* ======================== *
 * Functions for navigation *
 * ======================== */

/* Move to system i */
void execute_jump_to_planet(PlanetNum planetIndex)
{
	CurrentPlanet=planetIndex;
	LocalMarket = generate_market(random_byte(),Galaxy[planetIndex]);
}

/* Seperation between two planets (4*sqrt(X*X+Y*Y/4)) */
uint16_t distance(struct PlanSys systemA, struct PlanSys systemB)
{
	return (uint16_t)float_to_int_round(4*sqrt((systemA.x-systemB.x)*(systemA.x-systemB.x)+(systemA.y-systemB.y)*(systemA.y-systemB.y)/4));
}


/* Return id of the planet whose name matches passed strinmg
   closest to currentplanet - if none return currentplanet */
PlanetNum find_matching_system_name(char *searchName)
{
	PlanetNum syscount;
	PlanetNum p=CurrentPlanet;
	uint16_t d=9999;
	for(syscount=0;syscount<GAL_SIZE;++syscount)
	{ if (string_begins_with(searchName,Galaxy[syscount].name))
		{	if (distance(Galaxy[syscount],Galaxy[CurrentPlanet])<d)
			{ d=distance(Galaxy[syscount],Galaxy[CurrentPlanet]);
				p=syscount;
			}
		}
	}
	return p;
}


/* Print data for given system */
void print_system_info(struct PlanSys planetSystemInfo, bool useCompressedOutput)
{
	if (useCompressedOutput)
	{	
		//	  printf("\n ");
		printf("%10s",planetSystemInfo.name);
		printf(" TL: %2i ",(planetSystemInfo.techLev)+1);
		printf("%12s",EconNames[planetSystemInfo.economy]);
		printf(" %15s",GovNames[planetSystemInfo.govType]);
	}
	else
	{	printf("\n\nSystem:  ");
		printf("%s", planetSystemInfo.name);
		printf("\nPosition (%i,",planetSystemInfo.x);
		printf("%i)",planetSystemInfo.y);
		printf("\nEconomy: (%i) ",planetSystemInfo.economy);
		printf("%s", EconNames[planetSystemInfo.economy]);
		printf("\nGovernment: (%i) ",planetSystemInfo.govType);
		printf("%s", GovNames[planetSystemInfo.govType]);
		printf("\nTech Level: %2i",(planetSystemInfo.techLev)+1);
		printf("\nTurnover: %u",(planetSystemInfo.productivity));
		printf("\nRadius: %u",planetSystemInfo.radius);
		printf("\nPopulation: %u Billion",(planetSystemInfo.population)>>3);

		RndSeed = planetSystemInfo.goatSoupSeed;
		printf("\n");goat_soup("\x8F is \x97.",&planetSystemInfo);
	}
}

/* Various command functions */
bool do_tweak_random_native(char *commandArguments) 
{
	(void)commandArguments; // Mark 's' as unused
	NativeRand ^=1;
	return true;
}

bool do_local_systems_display(char *commandArguments)
{
	uint16_t d = (uint16_t)atoi(commandArguments);

	printf("Galaxy number %i",GalaxyNum);
	for(PlanetNum syscount=0; syscount < GAL_SIZE; ++syscount)
	{
		d=distance( Galaxy[syscount], Galaxy[CurrentPlanet] );

		if(d <= MaxFuel)
		{
			if( d <= Fuel )
				printf("\n * ");
			else
				printf("\n - ");

			print_system_info( Galaxy[syscount], true );
			printf(" (%.1f LY)", (float)d / 10);
		}
	}

	return true;
}


/* Jump to planet name s */
bool do_jump(char *commandArguments)
{
	uint16_t d;
	PlanetNum dest=find_matching_system_name(commandArguments);

	if(dest==CurrentPlanet)
	{
		printf("\nBad jump");
		return false;
	}

	d=distance(Galaxy[dest],Galaxy[CurrentPlanet]);

	if (d>Fuel)
	{
		printf("\nJump to far");
		return false;
	}

	Fuel-=d;
	execute_jump_to_planet(dest);
	print_system_info(Galaxy[CurrentPlanet],false);
	return true;
}

/* As dojump but no fuel cost */
bool do_sneak(char *commandArguments)
{
	uint16_t fuelkeep=Fuel;
	bool b;
	Fuel=666;
	b=do_jump(commandArguments);
	Fuel=fuelkeep;
	return b;
}


/* Jump to next galaxy */
bool do_galactic_hyperspace(char *commandArguments)
/*
 * Preserve planetnum (eg. if leave 7th planet
 * arrive at 7th planet) 
 * Classic Elite always jumped to planet nearest (0x60,0x60)
 */
{
	(void)(&commandArguments);     /* Discard s */
	GalaxyNum++;
	if(GalaxyNum==9) {GalaxyNum=1;}
	build_galaxy_data(GalaxyNum);
	return true;
}

/* Info on planet */
bool do_planet_info_display(char *commandArguments)
{
	PlanetNum dest=find_matching_system_name(commandArguments);
	print_system_info(Galaxy[dest],false);
	return true;
}


bool do_hold(char *commandArguments)
{
	uint16_t a=(uint16_t)atoi(commandArguments);
	uint16_t t=0;

	for(uint16_t i = 0; i <= LAST_TRADE; ++i)
	{
		if (( Commodities[i].units ) == TONNES)
			t += ShipHold[i];
	}

	if( t > a )
	{
		printf("\nHold too full");
		return false;
	}

	HoldSpace=a - t;

	return true;
}

/* Sell ammount S(2) of good S(1) */
bool do_sell(char *commandArguments)
{
	uint16_t i;
	uint16_t t;
	char s2[MAX_LEN];
	split_string_at_first_space(commandArguments,s2);
	uint16_t a = (uint16_t)atoi(commandArguments); // Correctly parse amount after splitting

	if (a==0)
		a=1;

	i=match_string_in_array(s2,tradnames,LAST_TRADE+1);

	if(i==0)
	{
		printf("\nUnknown trade good");
		return false;
	} 

	i-=1;

	t=execute_sell_order(i,a);

	if(t==0)
	{
		printf("Cannot sell any ");
	}
	else
	{	printf("\nSelling %i",t);
		printf("%s", UnitNames[Commodities[i].units]);
		printf(" of ");
	}

	printf("%s", tradnames[i]);

	return true;

}


/* Buy ammount S(2) of good S(1) */
bool do_buy(char *commandArguments)
{
	uint16_t i;
	uint16_t t;
	char s2[MAX_LEN];
	split_string_at_first_space(commandArguments,s2);
	uint16_t a = (uint16_t)atoi(commandArguments); // Correctly parse amount after splitting

	if (a==0) a=1;

	i=match_string_in_array(s2,tradnames,LAST_TRADE+1);

	if(i==0)
	{
		printf("\nUnknown trade good");
		return false;
	} 
	i-=1;

	t=execute_buy_order(i,a);
	if(t==0)
		printf("Cannot buy any ");
	else
	{
		printf("\nBuying %i",t);
		printf("%s", UnitNames[Commodities[i].units]);
		printf(" of ");
	}

	printf("%s", tradnames[i]);
	return true;
}

/* Attempt to buy f tonnes of fuel */
uint16_t calculate_fuel_purchase(uint16_t fuelAmount)
{
	if(fuelAmount+Fuel>MaxFuel)
		fuelAmount=MaxFuel-Fuel;

	if(FuelCost>0)
	{
		if((int)fuelAmount*FuelCost > Cash) 
			fuelAmount=(uint16_t)(Cash/FuelCost);
	}

	Fuel+=fuelAmount;
	Cash-=FuelCost*fuelAmount;

	return fuelAmount;
}


/* Buy ammount S of fuel */
bool do_fuel(char *commandArguments)
{
	uint16_t f=calculate_fuel_purchase((uint16_t)floor(10*atof(commandArguments)));
	if(f==0) { printf("\nCan't buy any fuel");}
	printf("\nBuying %.1fLY fuel",(float)f/10);
	return true;
}

/* Cheat alter cash by S */
bool do_cash(char *commandArguments)
{
	int a=(int)(10*atof(commandArguments));
	Cash+=(long)a;

	if(a != 0) 
		return true;

	printf("Number not understood");

	return false;
}

/* Show stock market */
bool do_market_display(char *commandArguments)
{
	(void)commandArguments; // Mark 's' as unused as the condition was always true
	// if((uint16_t)atoi(s) >= 0) // This condition is always true
	// {
		display_market_info(LocalMarket);

		printf("\nFuel :%.1f",(float)Fuel/10);
		printf("      Holdspace :%it",HoldSpace);
		return true;
	// }
	// else
	// 	return false; // This branch was unreachable
}

/* Obey command s */
bool parse_and_execute_command(char *commandString)
{
	uint16_t i;
	char c[MAX_LEN];
	commandString = strip_leading_trailing_spaces(commandString);
	if (strlen(commandString) == 0)
		return false;
	split_string_at_first_space(commandString,c);
	i=match_string_in_array(c,commands,NUM_COMMANDS);
	if(i)return (*comfuncs[i-1])(commandString) ;
	printf("\n Bad command (");
	printf("%s", c);
	printf(")");
	return false;
}


bool do_quit(char *commandArguments)
{
	(void)(&commandArguments);
	exit(ExitStatus);
	return ExitStatus == EXIT_SUCCESS ? true : false;
}

bool do_help(char *commandArguments)
{
	(void)(&commandArguments);
	printf("\nCommands are:");
	printf("\nBuy   tradegood ammount");
	printf("\nSell  tradegood ammount");
	printf("\nFuel  ammount    (buy ammount LY of fuel)");
	printf("\nJump  planetname (limited by fuel)");
	printf("\nSneak planetname (any distance - no fuel cost)");
	printf("\nGalhyp           (jumps to next galaxy)");
	printf("\nInfo  planetname (prints info on system");
	printf("\nMkt              (shows market prices)");
	printf("\nLocal            (lists systems within 7 light years)");
	printf("\nCash number      (alters cash - cheating!)");
	printf("\nHold number      (change cargo bay)");
	printf("\nQuit or ^C       (exit)");
	printf("\nHelp             (display this text)");
	printf("\nRand             (toggle RNG)");
	printf("\n\nAbbreviations allowed eg. b fo 5 = Buy Food 5, m= Mkt");
	return true;
}

int main()
{
	char getcommand[MAX_LEN];
	NativeRand=1;
	printf("\nWelcome to Text Elite 1.5.\n");

	for(uint16_t i = 0; i <= LAST_TRADE; i++)
		strcpy(tradnames[i], Commodities[i].name);

	my_srand(12345);/* Ensure repeatability */

	GalaxyNum=1;
	build_galaxy_data(GalaxyNum);

	CurrentPlanet=NUM_FOR_LAVE;                        /* Don't use jump */
	LocalMarket = generate_market(0x00,Galaxy[NUM_FOR_LAVE]);/* Since want seed=0 */

	Fuel=MaxFuel;

#define PARSER(S) { char buf[0x10]; strcpy(buf,S); parse_and_execute_command(buf); }   

	PARSER("hold 20");         /* Small cargo bay */
	PARSER("cash +100");       /* 100 CR */
	PARSER("help");

#undef PARSER

	for(;;)
	{
		printf("\n\nCash :%.1f>",((float)Cash)/10);
		if (!fgets(getcommand, sizeof(getcommand) - 1, stdin))
			break;
		getcommand[sizeof(getcommand) - 1] = '\0';
		parse_and_execute_command(getcommand);
	}

	printf("\n");

	exit(ExitStatus);
}

/* =============================================================================== *
 * "Goat Soup" planetary description string code - adapted from Christian Pinder's *
 *  reverse engineered sources.                                                    *
 * =============================================================================== */

struct DescChoice // Renamed from desc_choice
{
	const char *options[5]; // Renamed from option
};

static struct DescChoice descList[] = // Renamed from desc_list
{
/* 81 */	{{"fabled", "notable", "well known", "famous", "noted"}},
/* 82 */	{{"very", "mildly", "most", "reasonably", ""}},
/* 83 */	{{"ancient", "\x95", "great", "vast", "pink"}},
/* 84 */	{{"\x9E \x9D plantations", "mountains", "\x9C", "\x94 forests", "oceans"}},
/* 85 */	{{"shyness", "silliness", "mating traditions", "loathing of \x86", "love for \x86"}},
/* 86 */	{{"food blenders", "tourists", "poetry", "discos", "\x8E"}},
/* 87 */	{{"talking tree", "crab", "bat", "lobst", "\xB2"}},
/* 88 */	{{"beset", "plagued", "ravaged", "cursed", "scourged"}},
/* 89 */	{{"\x96 civil war", "\x9B \x98 \x99s", "a \x9B disease", "\x96 earthquakes", "\x96 solar activity"}},
/* 8A */	{{"its \x83 \x84", "the \xB1 \x98 \x99","its inhabitants' \x9A \x85", "\xA1", "its \x8D \x8E"}},
/* 8B */	{{"juice", "brandy", "water", "brew", "gargle blasters"}},
/* 8C */	{{"\xB2", "\xB1 \x99", "\xB1 \xB2", "\xB1 \x9B", "\x9B \xB2"}},
/* 8D */	{{"fabulous", "exotic", "hoopy", "unusual", "exciting"}},
/* 8E */	{{"cuisine", "night life", "casinos", "sit coms", " \xA1 "}},
/* 8F */	{{"\xB0", "The planet \xB0", "The world \xB0", "This planet", "This world"}},
/* 90 */	{{"n unremarkable", " boring", " dull", " tedious", " revolting"}},
/* 91 */	{{"planet", "world", "place", "little planet", "dump"}},
/* 92 */	{{"wasp", "moth", "grub", "ant", "\xB2"}},
/* 93 */	{{"poet", "arts graduate", "yak", "snail", "slug"}},
/* 94 */	{{"tropical", "dense", "rain", "impenetrable", "exuberant"}},
/* 95 */	{{"funny", "wierd", "unusual", "strange", "peculiar"}},
/* 96 */	{{"frequent", "occasional", "unpredictable", "dreadful", "deadly"}},
/* 97 */	{{"\x82 \x81 for \x8A", "\x82 \x81 for \x8A and \x8A", "\x88 by \x89", "\x82 \x81 for \x8A but \x88 by \x89","a\x90 \x91"}},
/* 98 */	{{"\x9B", "mountain", "edible", "tree", "spotted"}},
/* 99 */	{{"\x9F", "\xA0", "\x87oid", "\x93", "\x92"}},
/* 9A */	{{"ancient", "exceptional", "eccentric", "ingrained", "\x95"}},
/* 9B */	{{"killer", "deadly", "evil", "lethal", "vicious"}},
/* 9C */	{{"parking meters", "dust clouds", "ice bergs", "rock formations", "volcanoes"}},
/* 9D */	{{"plant", "tulip", "banana", "corn", "\xB2weed"}},
/* 9E */	{{"\xB2", "\xB1 \xB2", "\xB1 \x9B", "inhabitant", "\xB1 \xB2"}},
/* 9F */	{{"shrew", "beast", "bison", "snake", "wolf"}},
/* A0 */	{{"leopard", "cat", "monkey", "goat", "fish"}},
/* A1 */	{{"\x8C \x8B", "\xB1 \x9F \xA2","its \x8D \xA0 \xA2", "\xA3 \xA4", "\x8C \x8B"}},
/* A2 */	{{"meat", "cutlet", "steak", "burgers", "soup"}},
/* A3 */	{{"ice", "mud", "Zero-G", "vacuum", "\xB1 ultra"}},
/* A4 */	{{"hockey", "cricket", "karate", "polo", "tennis"}},
};

/* 
 * B0 = <planet name>
 * B1 = <planet name>ian
 * B2 = <random name>
 */
int gen_rnd_number (void)
{
	int a,x;
	x = (RndSeed.a * 2) & 0xFF;
	a = x + RndSeed.c;
	if (RndSeed.a > 127)	a++;
	RndSeed.a = a & 0xFF;
	RndSeed.c = x;

	a = a / 256;	/* a = any carry left from above */
	x = RndSeed.b;
	a = (a + x + RndSeed.d) & 0xFF;
	RndSeed.b = a;
	RndSeed.d = x;
	return a;
}


void goat_soup(const char *sourceString, struct PlanSys *planetSystem)
{
	for(;;)
	{	int c=*(sourceString++);
		/* Take just the lower byte of the character. Most C
		   implementations define char as signed by default; if we
		   don't do this then the special \x escapes above will be
		   interpreted as negative. */
		c &= 0xff;
		if(c=='\0')	break;
		if(c < 0x80) printf("%c",c);
		else
		{	if (c <= 0xA4)
			{	int rnd = gen_rnd_number();
				goat_soup(descList[c-0x81].options[(rnd >= 0x33)+(rnd >= 0x66)+(rnd >= 0x99)+(rnd >= 0xCC)],planetSystem);
			}
			else switch(c)
			{ case 0xB0: /* planet name */
				{ int i=1;
					printf("%c",planetSystem->name[0]);
					while(planetSystem->name[i]!='\0') printf("%c",tolower(planetSystem->name[i++]));
				}	break;
				case 0xB1: /* <planet name>ian */
				{ int i=1;
					printf("%c",planetSystem->name[0]);
					while(planetSystem->name[i]!='\0')
					{	if((planetSystem->name[i+1]!='\0') || ((planetSystem->name[i]!='E')	&& (planetSystem->name[i]!='I')))
						printf("%c",tolower(planetSystem->name[i]));
						i++;
					}
					printf("ian");
				}	break;
				case 0xB2: /* random name */




#if 1 // 1.5
				{
					int i;
					int len = gen_rnd_number() & 3;
					for (i = 0; i <= len; i++)
					{
						int x = gen_rnd_number() & 0x3e;
						if (i == 0)
						{
							printf("%c",pairs0[x]);
						}
						else
						{
							printf("%c",tolower(pairs0[x]));
						}

						printf("%c",tolower(pairs0[x+1]));

					} // endfor
				}
#else	// 1.4-



				{	int i;
					int len = gen_rnd_number() & 3;
					for(i=0;i<=len;i++)
					{	int x = gen_rnd_number() & 0x3e;
						if(pairs0[x]!='.') printf("%c",pairs0[x]);
						if(i && (pairs0[x+1]!='.')) printf("%c",pairs0[x+1]);
					}
				}
#endif				

				break;
				default: printf("<bad char in data [%X]>",c); return;
			}	/* endswitch */
		}	/* endelse */
	}	/* endwhile */
}	/* endfunc */

/**+end **/


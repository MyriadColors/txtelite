#ifndef ELITE_GALAXY_H
#define ELITE_GALAXY_H

#include "elite_state.h" // Unified header for constants, structures, and globals
#include "elite_utils.h" // For tweak_seed, strip_char_from_string

// Planet naming data
// Original pairs0, 1.5 planet names fix (conditional removed)
static const char pairs0[] =
"ABOUSEITILETSTONLONUTHNOALLEXEGEZACEBISOUSESARMAINDIREA.ERATENBERALAVETIEDORQUANTEISRION";

// Original pairs
static const char pairs[] = "..LEXEGEZACEBISO"
"USESARMAINDIREA."
"ERATENBERALAVETI"
"EDORQUANTEISRION"; /* Dots should be nullprint characters */

/* rotate 8 bit number leftwards */
static inline uint16_t rotate_left(uint16_t valueToRotate) 
{
	uint16_t temp = valueToRotate & 128;
	return (2 * (valueToRotate & 127)) + (temp >> 7);
} 

static inline uint16_t twist(uint16_t valueToTwist)
{
	return (uint16_t)((256 * rotate_left(valueToTwist >> 8)) + rotate_left(valueToTwist & 255));
}

/* Apply to base seed; once for galaxy 2 */
static inline void next_galaxy(struct SeedType *currentSeed)
{ 
	(*currentSeed).a = twist((*currentSeed).a);  /* twice for galaxy 3, etc. */
	(*currentSeed).b = twist((*currentSeed).b);  /* Eighth application gives galaxy 1 again*/
	(*currentSeed).c = twist((*currentSeed).c);
	(*currentSeed).d = twist((*currentSeed).d);  /* Added to handle d field */
}

/* Generate system info from seed */
// Operates on the global Seed, modifies it, and uses global Galaxy array.
// Depends on strip_char_from_string and tweak_seed from elite_utils.h
static inline struct PlanSys make_system(struct SeedType *initialSeed) // initialSeed is typically the global Seed
{
	struct PlanSys thissys;
	uint16_t pair1,pair2,pair3,pair4;
	uint16_t longnameflag=((*initialSeed).a)&64; // Changed w0 to a

	thissys.x=(((*initialSeed).b)>>8); // Changed w1 to b
	thissys.y=(((*initialSeed).a)>>8); // Changed w0 to a

	thissys.govType =((((*initialSeed).b)>>3)&7); /* bits 3,4 &5 of b (was w1) */

	thissys.economy =((((*initialSeed).a)>>8)&7); /* bits 8,9 &A of a (was w0) */
	if (thissys.govType <=1)
	{ thissys.economy = ((thissys.economy)|2);
	} 

	thissys.techLev =((((*initialSeed).b)>>8)&3)+((thissys.economy)^7); // Changed w1 to b
	thissys.techLev +=((thissys.govType)>>1);
	if (((thissys.govType)&1)==1)	thissys.techLev+=1;
	/* C simulation of 6502's LSR then ADC */

	thissys.population = 4*(thissys.techLev) + (thissys.economy);
	thissys.population +=  (thissys.govType) + 1;

	thissys.productivity = (((thissys.economy)^7)+3)*((thissys.govType)+4);
	thissys.productivity *= (thissys.population)*8;

	thissys.radius = 256*(((((*initialSeed).c)>>8)&15)+11) + thissys.x; // Changed w2 to c

	thissys.goatSoupSeed.a = (*initialSeed).b & 0xFF; // Changed w1 to b
	thissys.goatSoupSeed.b = (*initialSeed).b >>8;    // Changed w1 to b
	thissys.goatSoupSeed.c = (*initialSeed).c & 0xFF; // Changed w2 to c
	thissys.goatSoupSeed.d = (*initialSeed).c >> 8;   // Changed w2 to c

	pair1=2*((((*initialSeed).c)>>8)&31);  tweak_seed(initialSeed); // Changed w2 to c
	pair2=2*((((*initialSeed).c)>>8)&31);  tweak_seed(initialSeed); // Changed w2 to c
	pair3=2*((((*initialSeed).c)>>8)&31);  tweak_seed(initialSeed); // Changed w2 to c
	pair4=2*((((*initialSeed).c)>>8)&31); tweak_seed(initialSeed);  // Changed w2 to c
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

/* Original game generated from scratch each time info needed */
// Operates on global Seed and Galaxy array.
// Uses BASE_0, BASE_1, BASE_2 constants from elite_state.h
static inline void build_galaxy_data(struct SeedType seed)
{
	uint16_t syscount;
	Seed = seed; /* Use the seed passed as parameter */
	/* Put galaxy data into array of structures */  
	for(syscount=0;syscount<GAL_SIZE;++syscount) Galaxy[syscount]=make_system(&Seed);
}

#endif // ELITE_GALAXY_H

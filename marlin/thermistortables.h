#ifndef THERMISTORTABLES_H_
#define THERMISTORTABLES_H_

#include "Marlin.h"

#define OVERSAMPLENR 1
#if (THERMISTORHEATER == 1) || (THERMISTORBED == 1) //100k bed thermistor
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=4036 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 4036
// max adc: 1023
#define NUMTEMPS_1 256
const short temptable_1[NUMTEMPS_1][2] = {
  {1, 399}, //{1, 864},
  {5, 398}, // {5, 508},
   {9, 397}, //{9, 428},
   {13, 385},
   {17, 357},
   {21, 337},
   {25, 321},
   {29, 308},
   {33, 297},
   {37, 287},
   {41, 279},
   {45, 272},
   {49, 265},
   {53, 259},
   {57, 254},
   {61, 249},
   {65, 245},
   {69, 240},
   {73, 236},
   {77, 233},
   {81, 229},
   {85, 226},
   {89, 223},
   {93, 220},
   {97, 217},
   {101, 215},
   {105, 212},
   {109, 210},
   {113, 207},
   {117, 205},
   {121, 203},
   {125, 201},
   {129, 199},
   {133, 197},
   {137, 195},
   {141, 193},
   {145, 191},
   {149, 190},
   {153, 188},
   {157, 187},
   {161, 185},
   {165, 183},
   {169, 182},
   {173, 181},
   {177, 179},
   {181, 178},
   {185, 176},
   {189, 175},
   {193, 174},
   {197, 173},
   {201, 171},
   {205, 170},
   {209, 169},
   {213, 168},
   {217, 167},
   {221, 166},
   {225, 164},
   {229, 163},
   {233, 162},
   {237, 161},
   {241, 160},
   {245, 159},
   {249, 158},
   {253, 157},
   {257, 156},
   {261, 155},
   {265, 154},
   {269, 154},
   {273, 153},
   {277, 152},
   {281, 151},
   {285, 150},
   {289, 149},
   {293, 148},
   {297, 147},
   {301, 147},
   {305, 146},
   {309, 145},
   {313, 144},
   {317, 143},
   {321, 143},
   {325, 142},
   {329, 141},
   {333, 140},
   {337, 140},
   {341, 139},
   {345, 138},
   {349, 137},
   {353, 137},
   {357, 136},
   {361, 135},
   {365, 135},
   {369, 134},
   {373, 133},
   {377, 132},
   {381, 132},
   {385, 131},
   {389, 130},
   {393, 130},
   {397, 129},
   {401, 128},
   {405, 128},
   {409, 127},
   {413, 126},
   {417, 126},
   {421, 125},
   {425, 125},
   {429, 124},
   {433, 123},
   {437, 123},
   {441, 122},
   {445, 121},
   {449, 121},
   {453, 120},
   {457, 120},
   {461, 119},
   {465, 118},
   {469, 118},
   {473, 117},
   {477, 117},
   {481, 116},
   {485, 115},
   {489, 115},
   {493, 114},
   {497, 114},
   {501, 113},
   {505, 113},
   {509, 112},
   {513, 111},
   {517, 111},
   {521, 110},
   {525, 110},
   {529, 109},
   {533, 109},
   {537, 108},
   {541, 107},
   {545, 107},
   {549, 106},
   {553, 106},
   {557, 105},
   {561, 105},
   {565, 104},
   {569, 103},
   {573, 103},
   {577, 102},
   {581, 102},
   {585, 101},
   {589, 101},
   {593, 100},
   {597, 100},
   {601, 99},
   {605, 98},
   {609, 98},
   {613, 97},
   {617, 97},
   {621, 96},
   {625, 96},
   {629, 95},
   {633, 95},
   {637, 94},
   {641, 93},
   {645, 93},
   {649, 92},
   {653, 92},
   {657, 91},
   {661, 91},
   {665, 90},
   {669, 90},
   {673, 89},
   {677, 88},
   {681, 88},
   {685, 87},
   {689, 87},
   {693, 86},
   {697, 86},
   {701, 85},
   {705, 84},
   {709, 84},
   {713, 83},
   {717, 83},
   {721, 82},
   {725, 81},
   {729, 81},
   {733, 80},
   {737, 80},
   {741, 79},
   {745, 78},
   {749, 78},
   {753, 77},
   {757, 77},
   {761, 76},
   {765, 75},
   {769, 75},
   {773, 74},
   {777, 74},
   {781, 73},
   {785, 72},
   {789, 72},
   {793, 71},
   {797, 70},
   {801, 70},
   {805, 69},
   {809, 68},
   {813, 68},
   {817, 67},
   {821, 66},
   {825, 65},
   {829, 65},
   {833, 64},
   {837, 63},
   {841, 63},
   {845, 62},
   {849, 61},
   {853, 60},
   {857, 60},
   {861, 59},
   {865, 58},
   {869, 57},
   {873, 56},
   {877, 55},
   {881, 55},
   {885, 54},
   {889, 53},
   {893, 52},
   {897, 51},
   {901, 50},
   {905, 49},
   {909, 48},
   {913, 47},
   {917, 46},
   {921, 45},
   {925, 44},
   {929, 43},
   {933, 41},
   {937, 40},
   {941, 39},
   {945, 38},
   {949, 36},
   {953, 35},
   {957, 34},
   {961, 32},
   {965, 30},
   {969, 29},
   {973, 27},
   {977, 25},
   {981, 23},
   {985, 21},
   {989, 18},
   {993, 16},
   {997, 13},
   {1001, 10},
   {1005, 6},
   {1009, 1},
   {1013, 0},
   {1017, 0},
   {1021, 0}
};
#endif


#if (THERMISTORHEATER == 6) || (THERMISTORBED == 6) // 100k Epcos thermistor
#define NUMTEMPS_6 36
const short temptable_6[NUMTEMPS_6][2] = {
   {28, 250},
   {31, 245},
   {35, 240},
   {39, 235},
   {42, 230},
   {44, 225},
   {49, 220},
   {53, 215},
   {62, 210},
   {73, 205},
   {72, 200},
   {94, 190},
   {102, 185},
   {116, 170},
   {143, 160},
   {183, 150},
   {223, 140},
   {270, 130},
   {318, 120},
   {383, 110},
   {413, 105},
   {439, 100},
   {484, 95},
   {513, 90},
   {607, 80},
   {664, 70},
   {781, 60},
   {810, 55},
   {849, 50},
   {914, 45},
   {914, 40},
   {935, 35},
   {954, 30},
   {970, 25},
   {978, 22},
   {1008, 3}
};
#endif


#define _TT_NAME(_N) temptable_ ## _N
#define TT_NAME(_N) _TT_NAME(_N)

#ifdef THERMISTORHEATER_0
# define HEATER_0_TEMPTABLE TT_NAME(THERMISTORHEATER_0)
# define HEATER_0_TEMPTABLE_LEN (sizeof(HEATER_0_TEMPTABLE)/sizeof(*HEATER_0_TEMPTABLE))
#else
# ifdef HEATER_0_USES_THERMISTOR
#  error No heater 0 thermistor table specified
# else  // HEATER_0_USES_THERMISTOR
#  define HEATER_0_TEMPTABLE NULL
#  define HEATER_0_TEMPTABLE_LEN 0
# endif // HEATER_0_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_0_RAW_HI_TEMP
# ifdef HEATER_0_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_0_RAW_HI_TEMP 1023
#  define HEATER_0_RAW_LO_TEMP 0
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_0_RAW_HI_TEMP 1023
#  define HEATER_0_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORHEATER_1
# define HEATER_1_TEMPTABLE TT_NAME(THERMISTORHEATER_1)
# define HEATER_1_TEMPTABLE_LEN (sizeof(HEATER_1_TEMPTABLE)/sizeof(*HEATER_1_TEMPTABLE))
#else
# ifdef HEATER_1_USES_THERMISTOR
#  error No heater 1 thermistor table specified
# else  // HEATER_1_USES_THERMISTOR
#  define HEATER_1_TEMPTABLE NULL
#  define HEATER_1_TEMPTABLE_LEN 0
# endif // HEATER_1_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_1_RAW_HI_TEMP
# ifdef HEATER_1_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_1_RAW_HI_TEMP 0
#  define HEATER_1_RAW_LO_TEMP 16383
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_1_RAW_HI_TEMP 16383
#  define HEATER_1_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORHEATER_2
# define HEATER_2_TEMPTABLE TT_NAME(THERMISTORHEATER_2)
# define HEATER_2_TEMPTABLE_LEN (sizeof(HEATER_2_TEMPTABLE)/sizeof(*HEATER_2_TEMPTABLE))
#else
# ifdef HEATER_2_USES_THERMISTOR
#  error No heater 2 thermistor table specified
# else  // HEATER_2_USES_THERMISTOR
#  define HEATER_2_TEMPTABLE NULL
#  define HEATER_2_TEMPTABLE_LEN 0
# endif // HEATER_2_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_2_RAW_HI_TEMP
# ifdef HEATER_2_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_2_RAW_HI_TEMP 0
#  define HEATER_2_RAW_LO_TEMP 16383
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_2_RAW_HI_TEMP 16383
#  define HEATER_2_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORBED
# define BEDTEMPTABLE TT_NAME(THERMISTORBED)
# define BEDTEMPTABLE_LEN (sizeof(BEDTEMPTABLE)/sizeof(*BEDTEMPTABLE))
#else
# ifdef BED_USES_THERMISTOR
#  error No bed thermistor table specified
# endif // BED_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_BED_RAW_HI_TEMP
# ifdef BED_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_BED_RAW_HI_TEMP 1023
#  define HEATER_BED_RAW_LO_TEMP 0
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_BED_RAW_HI_TEMP 1023
#  define HEATER_BED_RAW_LO_TEMP 0
# endif
#endif





#if THERMISTORHEATER == 1
#define NUMTEMPS NUMTEMPS_1
#define temptable temptable_1
#elif THERMISTORHEATER == 2
#define NUMTEMPS NUMTEMPS_2
#define temptable temptable_2
#elif THERMISTORHEATER == 3
#define NUMTEMPS NUMTEMPS_3
#define temptable temptable_3
#elif THERMISTORHEATER == 4
#define NUMTEMPS NUMTEMPS_4
#define temptable temptable_4
#elif THERMISTORHEATER == 5
#define NUMTEMPS NUMTEMPS_5
#define temptable temptable_5
#elif THERMISTORHEATER == 6
#define NUMTEMPS NUMTEMPS_6
#define temptable temptable_6
#elif THERMISTORHEATER == 7
#define NUMTEMPS NUMTEMPS_7
#define temptable temptable_7
#elif defined HEATER_USES_THERMISTOR
#error No heater thermistor table specified
#endif
#if THERMISTORBED == 1
#define BNUMTEMPS NUMTEMPS_1
#define bedtemptable temptable_1
#elif THERMISTORBED == 2
#define BNUMTEMPS NUMTEMPS_2
#define bedtemptable temptable_2
#elif THERMISTORBED == 3
#define BNUMTEMPS NUMTEMPS_3
#define bedtemptable temptable_3
#elif THERMISTORBED == 4
#define BNUMTEMPS NUMTEMPS_4
#define bedtemptable temptable_4
#elif THERMISTORBED == 5
#define BNUMTEMPS NUMTEMPS_5
#define bedtemptable temptable_5
#elif THERMISTORBED == 6
#define BNUMTEMPS NUMTEMPS_6
#define bedtemptable temptable_6
#elif THERMISTORBED == 7
#define BNUMTEMPS NUMTEMPS_7
#define bedtemptable temptable_7
#elif defined BED_USES_THERMISTOR
#error No bed thermistor table specified
#endif

#endif //THERMISTORTABLES_H_

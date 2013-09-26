#ifndef SPEED_LOOKUPTABLE_H
#define SPEED_LOOKUPTABLE_H

#include "Marlin.h"

#if F_CPU == 96000000

const uint16_t speed_lookuptable_fast[256][2] = {
   {62500, 35715}, {26785, 9740}, {17045, 4545}, {12500, 2632}, {9868, 1716}, {8152, 1208}, {6944, 896}, {6048, 691},
   {5357, 550}, {4807, 447}, {4360, 371}, {3989, 313}, {3676, 267}, {3409, 232}, {3177, 201}, {2976, 178},
   {2798, 158}, {2640, 140}, {2500, 127}, {2373, 114}, {2259, 104}, {2155, 95}, {2060, 87}, {1973, 80},
   {1893, 73}, {1820, 68}, {1752, 63}, {1689, 59}, {1630, 55}, {1575, 51}, {1524, 48}, {1476, 45},
   {1431, 43}, {1388, 40}, {1348, 37}, {1311, 36}, {1275, 34}, {1241, 32}, {1209, 30}, {1179, 29},
   {1150, 28}, {1122, 26}, {1096, 25}, {1071, 24}, {1047, 23}, {1024, 22}, {1002, 21}, {981, 20},
   {961, 19}, {942, 19}, {923, 18}, {905, 17}, {888, 16}, {872, 16}, {856, 16}, {840, 15},
   {825, 14}, {811, 14}, {797, 13}, {784, 13}, {771, 12}, {759, 12}, {747, 12}, {735, 12},
   {723, 11}, {712, 10}, {702, 11}, {691, 10}, {681, 9}, {672, 10}, {662, 9}, {653, 9},
   {644, 9}, {635, 8}, {627, 9}, {618, 8}, {610, 8}, {602, 7}, {595, 8}, {587, 7},
   {580, 7}, {573, 7}, {566, 7}, {559, 6}, {553, 7}, {546, 6}, {540, 6}, {534, 6},
   {528, 6}, {522, 6}, {516, 6}, {510, 5}, {505, 5}, {500, 6}, {494, 5}, {489, 5},
   {484, 5}, {479, 5}, {474, 5}, {469, 4}, {465, 5}, {460, 4}, {456, 5}, {451, 4},
   {447, 4}, {443, 4}, {439, 4}, {435, 4}, {431, 4}, {427, 4}, {423, 4}, {419, 4},
   {415, 3}, {412, 4}, {408, 4}, {404, 3}, {401, 3}, {398, 4}, {394, 3}, {391, 3},
   {388, 3}, {385, 4}, {381, 3}, {378, 3}, {375, 3}, {372, 3}, {369, 3}, {366, 2},
   {364, 3}, {361, 3}, {358, 3}, {355, 2}, {353, 3}, {350, 3}, {347, 2}, {345, 3},
   {342, 2}, {340, 3}, {337, 2}, {335, 2}, {333, 3}, {330, 2}, {328, 2}, {326, 3},
   {323, 2}, {321, 2}, {319, 2}, {317, 2}, {315, 2}, {313, 3}, {310, 2}, {308, 2},
   {306, 2}, {304, 2}, {302, 2}, {300, 1}, {299, 2}, {297, 2}, {295, 2}, {293, 2},
   {291, 2}, {289, 1}, {288, 2}, {286, 2}, {284, 2}, {282, 1}, {281, 2}, {279, 2},
   {277, 1}, {276, 2}, {274, 2}, {272, 1}, {271, 2}, {269, 1}, {268, 2}, {266, 1},
   {265, 2}, {263, 1}, {262, 2}, {260, 1}, {259, 2}, {257, 1}, {256, 1}, {255, 2},
   {253, 1}, {252, 1}, {251, 2}, {249, 1}, {248, 1}, {247, 2}, {245, 1}, {244, 1},
   {243, 2}, {241, 1}, {240, 1}, {239, 1}, {238, 1}, {237, 2}, {235, 1}, {234, 1},
   {233, 1}, {232, 1}, {231, 1}, {230, 2}, {228, 1}, {227, 1}, {226, 1}, {225, 1},
   {224, 1}, {223, 1}, {222, 1}, {221, 1}, {220, 1}, {219, 1}, {218, 1}, {217, 1},
   {216, 1}, {215, 1}, {214, 1}, {213, 1}, {212, 1}, {211, 1}, {210, 1}, {209, 1},
   {208, 1}, {207, 1}, {206, 1}, {205, 1}, {204, 0}, {204, 1}, {203, 1}, {202, 1},
   {201, 1}, {200, 1}, {199, 1}, {198, 1}, {197, 0}, {197, 1}, {196, 1}, {195, 1},
   {194, 1}, {193, 0}, {193, 1}, {192, 1}, {191, 1}, {190, 1}, {189, 0}, {189, 1},
   {188, 1}, {187, 1}, {186, 0}, {186, 1}, {185, 1}, {184, 0}, {184, 1}, {183, 1},
};

const uint16_t speed_lookuptable_slow[256][2] = {
   {62500, 2500}, {60000, 2308}, {57692, 2137}, {55555, 1984}, {53571, 1847}, {51724, 1724}, {50000, 1613}, {48387, 1512},
   {46875, 1421}, {45454, 1337}, {44117, 1260}, {42857, 1191}, {41666, 1126}, {40540, 1067}, {39473, 1012}, {38461, 961},
   {37500, 915}, {36585, 871}, {35714, 831}, {34883, 793}, {34090, 757}, {33333, 725}, {32608, 694}, {31914, 664},
   {31250, 638}, {30612, 612}, {30000, 589}, {29411, 565}, {28846, 545}, {28301, 524}, {27777, 505}, {27272, 487},
   {26785, 470}, {26315, 453}, {25862, 439}, {25423, 423}, {25000, 410}, {24590, 397}, {24193, 384}, {23809, 372},
   {23437, 361}, {23076, 349}, {22727, 339}, {22388, 330}, {22058, 319}, {21739, 311}, {21428, 302}, {21126, 293},
   {20833, 286}, {20547, 277}, {20270, 270}, {20000, 264}, {19736, 256}, {19480, 250}, {19230, 243}, {18987, 237},
   {18750, 232}, {18518, 226}, {18292, 220}, {18072, 215}, {17857, 210}, {17647, 206}, {17441, 200}, {17241, 196},
   {17045, 192}, {16853, 187}, {16666, 183}, {16483, 179}, {16304, 175}, {16129, 172}, {15957, 168}, {15789, 164},
   {15625, 162}, {15463, 157}, {15306, 155}, {15151, 151}, {15000, 149}, {14851, 146}, {14705, 142}, {14563, 140},
   {14423, 138}, {14285, 135}, {14150, 132}, {14018, 130}, {13888, 127}, {13761, 125}, {13636, 123}, {13513, 121},
   {13392, 118}, {13274, 117}, {13157, 114}, {13043, 112}, {12931, 111}, {12820, 109}, {12711, 106}, {12605, 105},
   {12500, 104}, {12396, 101}, {12295, 100}, {12195, 99}, {12096, 96}, {12000, 96}, {11904, 93}, {11811, 93},
   {11718, 91}, {11627, 89}, {11538, 88}, {11450, 87}, {11363, 85}, {11278, 84}, {11194, 83}, {11111, 82},
   {11029, 81}, {10948, 79}, {10869, 78}, {10791, 77}, {10714, 76}, {10638, 75}, {10563, 74}, {10489, 73},
   {10416, 72}, {10344, 71}, {10273, 69}, {10204, 69}, {10135, 68}, {10067, 67}, {10000, 67}, {9933, 65},
   {9868, 65}, {9803, 63}, {9740, 63}, {9677, 62}, {9615, 61}, {9554, 61}, {9493, 60}, {9433, 58},
   {9375, 59}, {9316, 57}, {9259, 57}, {9202, 56}, {9146, 56}, {9090, 54}, {9036, 54}, {8982, 54},
   {8928, 53}, {8875, 52}, {8823, 52}, {8771, 51}, {8720, 50}, {8670, 50}, {8620, 49}, {8571, 49},
   {8522, 48}, {8474, 48}, {8426, 47}, {8379, 46}, {8333, 46}, {8287, 46}, {8241, 45}, {8196, 44},
   {8152, 44}, {8108, 44}, {8064, 43}, {8021, 43}, {7978, 42}, {7936, 42}, {7894, 41}, {7853, 41},
   {7812, 40}, {7772, 41}, {7731, 39}, {7692, 39}, {7653, 39}, {7614, 39}, {7575, 38}, {7537, 37},
   {7500, 38}, {7462, 37}, {7425, 36}, {7389, 37}, {7352, 35}, {7317, 36}, {7281, 35}, {7246, 35},
   {7211, 34}, {7177, 35}, {7142, 33}, {7109, 34}, {7075, 33}, {7042, 33}, {7009, 33}, {6976, 32},
   {6944, 32}, {6912, 32}, {6880, 31}, {6849, 31}, {6818, 31}, {6787, 31}, {6756, 30}, {6726, 30},
   {6696, 30}, {6666, 29}, {6637, 30}, {6607, 29}, {6578, 28}, {6550, 29}, {6521, 28}, {6493, 28},
   {6465, 28}, {6437, 27}, {6410, 28}, {6382, 27}, {6355, 26}, {6329, 27}, {6302, 26}, {6276, 26},
   {6250, 26}, {6224, 26}, {6198, 26}, {6172, 25}, {6147, 25}, {6122, 25}, {6097, 25}, {6072, 24},
   {6048, 24}, {6024, 24}, {6000, 24}, {5976, 24}, {5952, 24}, {5928, 23}, {5905, 23}, {5882, 23},
   {5859, 23}, {5836, 23}, {5813, 22}, {5791, 22}, {5769, 22}, {5747, 22}, {5725, 22}, {5703, 22},
   {5681, 21}, {5660, 21}, {5639, 22}, {5617, 20}, {5597, 21}, {5576, 21}, {5555, 20}, {5535, 21},
   {5514, 20}, {5494, 20}, {5474, 20}, {5454, 20}, {5434, 19}, {5415, 20}, {5395, 19}, {5376, 19},
};

#endif
#endif

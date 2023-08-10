#ifndef SYNTH_WAVEFORM_
#define SYNTH_WAVEFORM_

#include "audiocontext.h"

// TODO: DRY wrt filter.h & why can't I use math.h for this?
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define WAVE_TABLE_LENGTH 512
#define FIX16_WAVE_TABLE_LENGTH 33554432

/*! \brief Generate an audio sample
 *
 * \param context The audio context
 */
fix16 synth_waveform_sample(Voice_t* context);

static fix16 SineWaveTable[WAVE_TABLE_LENGTH] = {
    0,      804,    1608,   2412,   3215,   4018,   4821,   5622,   6423,
    7223,   8022,   8819,   9616,   10410,  11204,  11995,  12785,  13573,
    14359,  15142,  15923,  16702,  17479,  18253,  19024,  19792,  20557,
    21319,  22078,  22833,  23586,  24334,  25079,  25820,  26557,  27291,
    28020,  28745,  29465,  30181,  30893,  31600,  32302,  32999,  33692,
    34379,  35061,  35738,  36409,  37075,  37736,  38390,  39039,  39682,
    40319,  40950,  41575,  42194,  42806,  43412,  44011,  44603,  45189,
    45768,  46340,  46906,  47464,  48015,  48558,  49095,  49624,  50146,
    50660,  51166,  51665,  52155,  52639,  53114,  53581,  54040,  54491,
    54933,  55368,  55794,  56212,  56621,  57022,  57414,  57797,  58172,
    58538,  58895,  59243,  59583,  59913,  60235,  60547,  60850,  61144,
    61429,  61705,  61971,  62228,  62475,  62714,  62942,  63162,  63371,
    63571,  63762,  63943,  64115,  64276,  64428,  64571,  64703,  64826,
    64939,  65043,  65136,  65220,  65294,  65358,  65412,  65457,  65491,
    65516,  65531,  65535,  65531,  65516,  65491,  65457,  65412,  65358,
    65294,  65220,  65136,  65043,  64939,  64826,  64703,  64571,  64428,
    64276,  64115,  63943,  63762,  63571,  63371,  63162,  62942,  62714,
    62475,  62228,  61971,  61705,  61429,  61144,  60850,  60547,  60235,
    59913,  59583,  59243,  58895,  58538,  58172,  57797,  57414,  57022,
    56621,  56212,  55794,  55368,  54933,  54491,  54040,  53581,  53114,
    52639,  52155,  51665,  51166,  50660,  50146,  49624,  49095,  48558,
    48015,  47464,  46906,  46340,  45768,  45189,  44603,  44011,  43412,
    42806,  42194,  41575,  40950,  40319,  39682,  39039,  38390,  37736,
    37075,  36409,  35738,  35061,  34379,  33692,  32999,  32302,  31600,
    30893,  30181,  29465,  28745,  28020,  27291,  26557,  25820,  25079,
    24334,  23586,  22833,  22078,  21319,  20557,  19792,  19024,  18253,
    17479,  16702,  15923,  15142,  14359,  13573,  12785,  11995,  11204,
    10410,  9616,   8819,   8022,   7223,   6423,   5622,   4821,   4018,
    3215,   2412,   1608,   804,    0,      -804,   -1608,  -2412,  -3215,
    -4018,  -4821,  -5622,  -6423,  -7223,  -8022,  -8819,  -9616,  -10410,
    -11204, -11995, -12785, -13573, -14359, -15142, -15923, -16702, -17479,
    -18253, -19024, -19792, -20557, -21319, -22078, -22833, -23586, -24334,
    -25079, -25820, -26557, -27291, -28020, -28745, -29465, -30181, -30893,
    -31600, -32302, -32999, -33692, -34379, -35061, -35738, -36409, -37075,
    -37736, -38390, -39039, -39682, -40319, -40950, -41575, -42194, -42806,
    -43412, -44011, -44603, -45189, -45768, -46340, -46906, -47464, -48015,
    -48558, -49095, -49624, -50145, -50660, -51166, -51665, -52155, -52639,
    -53114, -53581, -54040, -54491, -54933, -55368, -55794, -56212, -56621,
    -57022, -57414, -57797, -58172, -58538, -58895, -59243, -59583, -59913,
    -60235, -60547, -60850, -61144, -61429, -61705, -61971, -62228, -62475,
    -62714, -62942, -63162, -63371, -63571, -63762, -63943, -64115, -64276,
    -64428, -64571, -64703, -64826, -64939, -65043, -65136, -65220, -65294,
    -65358, -65412, -65457, -65491, -65516, -65531, -65535, -65531, -65516,
    -65491, -65457, -65412, -65358, -65294, -65220, -65136, -65043, -64939,
    -64826, -64703, -64571, -64428, -64276, -64115, -63943, -63762, -63571,
    -63371, -63162, -62942, -62714, -62475, -62228, -61971, -61705, -61429,
    -61144, -60850, -60547, -60235, -59913, -59583, -59243, -58895, -58538,
    -58172, -57797, -57414, -57022, -56621, -56212, -55794, -55368, -54933,
    -54491, -54040, -53581, -53114, -52639, -52155, -51665, -51166, -50660,
    -50145, -49624, -49095, -48558, -48015, -47464, -46906, -46340, -45768,
    -45189, -44603, -44011, -43412, -42806, -42194, -41575, -40950, -40319,
    -39682, -39039, -38390, -37736, -37075, -36409, -35738, -35061, -34379,
    -33692, -32999, -32302, -31600, -30893, -30181, -29465, -28745, -28020,
    -27291, -26557, -25820, -25079, -24334, -23586, -22833, -22078, -21319,
    -20557, -19792, -19024, -18253, -17479, -16702, -15923, -15142, -14359,
    -13573, -12785, -11995, -11204, -10410, -9616,  -8819,  -8022,  -7223,
    -6423,  -5622,  -4821,  -4018,  -3215,  -2412,  -1608,  -804};

static fix16 TriangleWaveTable[WAVE_TABLE_LENGTH] = {
    -65536, -65024, -64512, -64000, -63488, -62976, -62464, -61952, -61440,
    -60928, -60416, -59904, -59392, -58880, -58368, -57856, -57344, -56832,
    -56320, -55808, -55296, -54784, -54272, -53760, -53248, -52736, -52224,
    -51712, -51200, -50688, -50176, -49664, -49152, -48640, -48128, -47616,
    -47104, -46592, -46080, -45568, -45056, -44544, -44032, -43520, -43008,
    -42496, -41984, -41472, -40960, -40448, -39936, -39424, -38912, -38400,
    -37888, -37376, -36864, -36352, -35840, -35328, -34816, -34304, -33792,
    -33280, -32768, -32256, -31744, -31232, -30720, -30208, -29696, -29184,
    -28672, -28160, -27648, -27136, -26624, -26112, -25600, -25088, -24576,
    -24064, -23552, -23040, -22528, -22016, -21504, -20992, -20480, -19968,
    -19456, -18944, -18432, -17920, -17408, -16896, -16384, -15872, -15360,
    -14848, -14336, -13824, -13312, -12800, -12288, -11776, -11264, -10752,
    -10240, -9728,  -9216,  -8704,  -8192,  -7680,  -7168,  -6656,  -6144,
    -5632,  -5120,  -4608,  -4096,  -3584,  -3072,  -2560,  -2048,  -1536,
    -1024,  -512,   0,      512,    1024,   1536,   2048,   2560,   3072,
    3584,   4096,   4608,   5120,   5632,   6144,   6656,   7168,   7680,
    8192,   8704,   9216,   9728,   10240,  10752,  11264,  11776,  12288,
    12800,  13312,  13824,  14336,  14848,  15360,  15872,  16384,  16896,
    17408,  17920,  18432,  18944,  19456,  19968,  20480,  20992,  21504,
    22016,  22528,  23040,  23552,  24064,  24576,  25088,  25600,  26112,
    26624,  27136,  27648,  28160,  28672,  29184,  29696,  30208,  30720,
    31232,  31744,  32256,  32768,  33280,  33792,  34304,  34816,  35328,
    35840,  36352,  36864,  37376,  37888,  38400,  38912,  39424,  39936,
    40448,  40960,  41472,  41984,  42496,  43008,  43520,  44032,  44544,
    45056,  45568,  46080,  46592,  47104,  47616,  48128,  48640,  49152,
    49664,  50176,  50688,  51200,  51712,  52224,  52736,  53248,  53760,
    54272,  54784,  55296,  55808,  56320,  56832,  57344,  57856,  58368,
    58880,  59392,  59904,  60416,  60928,  61440,  61952,  62464,  62976,
    63488,  64000,  64512,  65024,  65536,  65024,  64512,  64000,  63488,
    62976,  62464,  61952,  61440,  60928,  60416,  59904,  59392,  58880,
    58368,  57856,  57344,  56832,  56320,  55808,  55296,  54784,  54272,
    53760,  53248,  52736,  52224,  51712,  51200,  50688,  50176,  49664,
    49152,  48640,  48128,  47616,  47104,  46592,  46080,  45568,  45056,
    44544,  44032,  43520,  43008,  42496,  41984,  41472,  40960,  40448,
    39936,  39424,  38912,  38400,  37888,  37376,  36864,  36352,  35840,
    35328,  34816,  34304,  33792,  33280,  32768,  32256,  31744,  31232,
    30720,  30208,  29696,  29184,  28672,  28160,  27648,  27136,  26624,
    26112,  25600,  25088,  24576,  24064,  23552,  23040,  22528,  22016,
    21504,  20992,  20480,  19968,  19456,  18944,  18432,  17920,  17408,
    16896,  16384,  15872,  15360,  14848,  14336,  13824,  13312,  12800,
    12288,  11776,  11264,  10752,  10240,  9728,   9216,   8704,   8192,
    7680,   7168,   6656,   6144,   5632,   5120,   4608,   4096,   3584,
    3072,   2560,   2048,   1536,   1024,   512,    0,      -512,   -1024,
    -1536,  -2048,  -2560,  -3072,  -3584,  -4096,  -4608,  -5120,  -5632,
    -6144,  -6656,  -7168,  -7680,  -8192,  -8704,  -9216,  -9728,  -10240,
    -10752, -11264, -11776, -12288, -12800, -13312, -13824, -14336, -14848,
    -15360, -15872, -16384, -16896, -17408, -17920, -18432, -18944, -19456,
    -19968, -20480, -20992, -21504, -22016, -22528, -23040, -23552, -24064,
    -24576, -25088, -25600, -26112, -26624, -27136, -27648, -28160, -28672,
    -29184, -29696, -30208, -30720, -31232, -31744, -32256, -32768, -33280,
    -33792, -34304, -34816, -35328, -35840, -36352, -36864, -37376, -37888,
    -38400, -38912, -39424, -39936, -40448, -40960, -41472, -41984, -42496,
    -43008, -43520, -44032, -44544, -45056, -45568, -46080, -46592, -47104,
    -47616, -48128, -48640, -49152, -49664, -50176, -50688, -51200, -51712,
    -52224, -52736, -53248, -53760, -54272, -54784, -55296, -55808, -56320,
    -56832, -57344, -57856, -58368, -58880, -59392, -59904, -60416, -60928,
    -61440, -61952, -62464, -62976, -63488, -64000, -64512, -65024};

static fix16 SawWaveTable[WAVE_TABLE_LENGTH] = {
    0,      1306,   2612,   3915,   5214,   6509,   7798,   9080,   10353,
    11617,  12871,  14113,  15342,  16557,  17757,  18942,  20110,  21259,
    22390,  23501,  24592,  25661,  26708,  27731,  28731,  29706,  30655,
    31579,  32476,  33347,  34189,  35003,  35789,  36546,  37273,  37971,
    38638,  39276,  39883,  40460,  41006,  41521,  42006,  42460,  42883,
    43276,  43638,  43970,  44272,  44545,  44788,  45001,  45187,  45343,
    45472,  45574,  45649,  45697,  45719,  45717,  45689,  45638,  45563,
    45466,  45347,  45206,  45045,  44865,  44665,  44447,  44212,  43960,
    43693,  43410,  43114,  42804,  42482,  42148,  41803,  41448,  41084,
    40711,  40331,  39944,  39551,  39153,  38750,  38344,  37934,  37522,
    37108,  36694,  36279,  35864,  35450,  35038,  34627,  34220,  33815,
    33414,  33017,  32625,  32237,  31855,  31478,  31107,  30743,  30385,
    30033,  29689,  29352,  29022,  28700,  28385,  28078,  27779,  27488,
    27204,  26928,  26660,  26399,  26146,  25901,  25663,  25432,  25207,
    24990,  24780,  24575,  24377,  24185,  23999,  23818,  23642,  23471,
    23304,  23142,  22983,  22828,  22676,  22527,  22380,  22236,  22094,
    21953,  21813,  21674,  21536,  21398,  21260,  21121,  20982,  20841,
    20700,  20556,  20411,  20264,  20114,  19962,  19807,  19649,  19488,
    19323,  19155,  18983,  18808,  18628,  18444,  18257,  18065,  17869,
    17668,  17464,  17255,  17042,  16824,  16603,  16377,  16147,  15914,
    15676,  15435,  15190,  14941,  14689,  14435,  14177,  13916,  13653,
    13387,  13120,  12850,  12579,  12306,  12032,  11757,  11481,  11205,
    10928,  10652,  10375,  10100,  9825,   9551,   9278,   9007,   8738,
    8471,   8206,   7943,   7683,   7427,   7173,   6922,   6675,   6431,
    6192,   5956,   5724,   5497,   5274,   5055,   4841,   4631,   4426,
    4226,   4031,   3840,   3654,   3473,   3296,   3125,   2957,   2795,
    2637,   2483,   2334,   2189,   2048,   1911,   1777,   1648,   1521,
    1398,   1278,   1161,   1047,   935,    826,    718,    612,    507,
    404,    302,    201,    100,    0,      -100,   -201,   -302,   -404,
    -507,   -612,   -718,   -826,   -935,   -1047,  -1161,  -1278,  -1398,
    -1521,  -1648,  -1777,  -1911,  -2048,  -2189,  -2334,  -2483,  -2637,
    -2795,  -2957,  -3125,  -3296,  -3473,  -3654,  -3840,  -4031,  -4226,
    -4426,  -4631,  -4841,  -5055,  -5274,  -5497,  -5724,  -5956,  -6192,
    -6431,  -6675,  -6922,  -7173,  -7427,  -7683,  -7943,  -8206,  -8471,
    -8738,  -9007,  -9278,  -9551,  -9825,  -10100, -10375, -10652, -10928,
    -11205, -11481, -11757, -12032, -12306, -12579, -12850, -13120, -13387,
    -13653, -13916, -14177, -14435, -14689, -14941, -15190, -15435, -15676,
    -15914, -16147, -16377, -16603, -16824, -17042, -17255, -17464, -17668,
    -17869, -18065, -18257, -18444, -18628, -18808, -18983, -19155, -19323,
    -19488, -19649, -19807, -19962, -20114, -20264, -20411, -20556, -20700,
    -20841, -20982, -21121, -21260, -21398, -21536, -21674, -21813, -21953,
    -22094, -22236, -22380, -22527, -22676, -22828, -22983, -23142, -23304,
    -23471, -23642, -23818, -23999, -24185, -24377, -24576, -24780, -24990,
    -25207, -25432, -25663, -25901, -26146, -26399, -26660, -26928, -27204,
    -27488, -27779, -28078, -28385, -28700, -29022, -29352, -29689, -30033,
    -30385, -30743, -31107, -31478, -31855, -32237, -32625, -33017, -33414,
    -33815, -34220, -34627, -35038, -35450, -35864, -36279, -36694, -37108,
    -37522, -37934, -38344, -38750, -39153, -39551, -39944, -40331, -40711,
    -41084, -41448, -41803, -42148, -42482, -42804, -43114, -43410, -43693,
    -43960, -44212, -44447, -44665, -44865, -45045, -45206, -45347, -45466,
    -45563, -45638, -45689, -45717, -45719, -45697, -45649, -45574, -45472,
    -45343, -45187, -45001, -44788, -44545, -44272, -43970, -43638, -43276,
    -42883, -42460, -42006, -41521, -41006, -40460, -39883, -39276, -38638,
    -37971, -37273, -36546, -35789, -35003, -34189, -33346, -32476, -31579,
    -30655, -29706, -28731, -27731, -26708, -25661, -24592, -23501, -22390,
    -21259, -20110, -18942, -17757, -16557, -15342, -14113, -12871, -11617,
    -10353, -9080,  -7798,  -6509,  -5214,  -3915,  -2612,  -1306};

#endif
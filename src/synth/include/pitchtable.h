#ifndef SYNTH_PITCHTABLE_
#define SYNTH_PITCHTABLE_

#include <stdlib.h>
#include "pico/stdlib.h"

float frequency_from_reference_pitch(float f, uint8_t pitch);

#define PITCH_C0 32.703
#define PITCH_Db0 34.648
#define PITCH_D0 36.708
#define PITCH_Eb0 38.891
#define PITCH_E0 41.203
#define PITCH_F0 43.654
#define PITCH_Gb0 46.249
#define PITCH_G0 48.999
#define PITCH_Ab1 51.913
#define PITCH_A1 55
#define PITCH_Bb1 58.27
#define PITCH_B1 61.735

#define PITCH_C1 65.406
#define PITCH_Db1 69.296
#define PITCH_D1 73.416
#define PITCH_Eb1 77.782
#define PITCH_E1 82.407
#define PITCH_F1 87.307
#define PITCH_Gb1 92.499
#define PITCH_G1 97.999
#define PITCH_Ab2 103.826
#define PITCH_A2 110
#define PITCH_Bb2 116.541
#define PITCH_B2 123.471

#define PITCH_C2 130.813
#define PITCH_Db2 138.591
#define PITCH_D2 146.832
#define PITCH_Eb2 155.563
#define PITCH_E2 164.814
#define PITCH_F2 174.614
#define PITCH_Gb2 184.997
#define PITCH_G2 195.998
#define PITCH_Ab3 207.652
#define PITCH_A3 220
#define PITCH_Bb3 233.082
#define PITCH_B3 246.942

#define PITCH_C3 261.626
#define PITCH_Db3 277.183
#define PITCH_D3 293.665
#define PITCH_Eb3 311.127
#define PITCH_E3 329.628
#define PITCH_F3 349.228
#define PITCH_Gb3 369.994
#define PITCH_G3 391.995
#define PITCH_Ab4 415.305
#define PITCH_A4 440
#define PITCH_Bb4 466.164
#define PITCH_B4 493.883

#define PITCH_C4 523.251
#define PITCH_Db4 554.365
#define PITCH_D4 587.33
#define PITCH_Eb4 622.254
#define PITCH_E4 659.255
#define PITCH_F4 698.456
#define PITCH_Gb4 739.989
#define PITCH_G4 783.991
#define PITCH_Ab5 830.609
#define PITCH_A5 880
#define PITCH_Bb5 932.328
#define PITCH_B5 987.767

#define PITCH_C5 1046.502
#define PITCH_Db5 1108.731
#define PITCH_D5 1174.659
#define PITCH_Eb5 1244.508
#define PITCH_E5 1318.51
#define PITCH_F5 1396.913
#define PITCH_Gb5 1479.978
#define PITCH_G5 1567.982
#define PITCH_Ab6 1661.219
#define PITCH_A6 1760
#define PITCH_Bb6 1864.655
#define PITCH_B6 1975.533

#define PITCH_C6 2093.005
#define PITCH_Db6 2217.461
#define PITCH_D6 2349.318
#define PITCH_Eb6 2489.016
#define PITCH_E6 2637.021
#define PITCH_F6 2793.826
#define PITCH_Gb6 2959.955
#define PITCH_G6 3135.964
#define PITCH_Ab7 3322.438
#define PITCH_A7 3520
#define PITCH_Bb7 3729.31
#define PITCH_B7 3951.066

#define PITCH_C7 4186.009
#define PITCH_Db7 4434.922
#define PITCH_D7 4698.636
#define PITCH_Eb7 4978.032
#define PITCH_E7 5274.042
#define PITCH_F7 5587.652
#define PITCH_Gb7 5919.91
#define PITCH_G7 6271.928
#define PITCH_Ab8 6644.876
#define PITCH_A8 7040
#define PITCH_Bb8 7458.62
#define PITCH_B8 7902.132

#define PITCH_C8 8372.018
#define PITCH_Db8 8869.844
#define PITCH_D8 9397.272
#define PITCH_Eb8 9956.064
#define PITCH_E8 10548.084
#define PITCH_F8 11175.304
#define PITCH_Gb8 11839.82
#define PITCH_G8 12543.856
#define PITCH_Ab9 13289.752
#define PITCH_A9 14080
#define PITCH_Bb9 14917.24
#define PITCH_B9 15804.264

#endif
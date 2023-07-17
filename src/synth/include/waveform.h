#ifndef SYNTH_WAVEFORM_
#define SYNTH_WAVEFORM_

#include "../include/audiocontext.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif

#ifndef TWOPI
#define TWOPI		6.28318530717958647692	/* pi*2 */
#endif

/*! \brief Generate an audio sample
 *
 * \param context The audio context
 * \param waveform The waveform to use
 * \param frequency Frequency in hertz
 * \param amplitude between 0.0 and 1.0, where 1.0 is maximum volume
 */
double synth_waveform_sample(AudioContext_t *context);

#endif
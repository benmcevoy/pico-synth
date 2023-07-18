# ﻿pico-synth

DONE - solder on some headers

DONE - add a reset button pin 30 short to ground

## DONE - toolchain



installed mostly as per 

`https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh`

skipped OpenOCD and some pi related things


blink - it sure did

hello_world

`sudo dmesg`

shows the usb device

then we can monitor it

`sudo plink -serial /dev/ttyACM0 -sercfg 115200`

## CMAKE Build

if you just run cmake it makes an awful mess

i don't really know what i'm doing...

grabbed the exmaple base cmake.txt and sdk import files as the basis

set the project to synth

in the synth project just include main.c to build, plus a bit of pico config

e.g.

```
pico_enable_stdio_usb(synth 1)
pico_enable_stdio_uart(synth 0)
```


prepare the build folder, from /src

```
mkdir build
cd build
cmake ..
```

this dumps a load of stuff in build

then cd into e.g. build/synth and

`make -j4`

or just `make`


cmake makes make.  duh...


## logging via uart

currently logging via usb.  i read that this is intensive compared to UART.

just turn on UART and turn off USB for stdio

```
pico_enable_stdio_usb(${projectName} 0)
pico_enable_stdio_uart(${projectName} 1)
```

for now, don't care.  it's more convient to use usb.

`sudo plink -serial /dev/ttyACM0 -sercfg 115200`

## wavetable on an irq

i have a header file from circle that has a sample in  it... let's nick it.

i assume that once started i have to go into some loop to stay alive...  what is the basic pattern here?

"The return code of main() is ignored by the SDK runtime, and the default behaviour is to hang the processor on exit."

so we can just loop through the sound.h array then exit.  no wukkas.

```
// set up pwm
// setup an irq to provide a nice clock
// the sample is 44.1kHz and 8 bit
```

pointer into an array is going to be *char++ something like that... to step 8 bits/byte per step


some code from the pico example:

```c
// Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);

    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 3);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
```

**what is a slice?**

in the sdk docs and the datasheet it is not defined.  but i think that PWM has a master clock then subdivides that up into 8 or 16 "slices"? so each slice has some clock to tell it when to go high, go low.

https://www.codrey.com/raspberry-pi/raspberry-pi-pico-pwm-primer/

so, pin0 has associated slice number 0A  that's not HEX mind, that's 0 channel A

and pin1 is channel b e.g. 0B

so we do get 16 "channels" on 8 "slices"


**what is a wrap?**

guessing this is when the pwm counter wraps?

it's also not defined, thanks for that.

looks like

```
channelA: -___-___-___
channelb: ---_---_---_
```

so this example is not really what we want.

https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#hardware_pwm

this is better but still assumes an awful lot...

https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#detailed-description109

ok rtfm...

another example ticks some boxes...

```c
    // Tell the LED pin that the PWM is in charge of its value.
    gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
    // Figure out which slice we just connected to the LED pin
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);

    // Mask our slice's IRQ output into the PWM block's single interrupt line,
    // and register our interrupt handler
    pwm_clear_irq(slice_num);
    pwm_set_irq_enabled(slice_num, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // Get some sensible defaults for the slice configuration. By default, the
    // counter is allowed to wrap over its maximum range (0 to 2**16-1)
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);

    // Everything after this point happens in the PWM interrupt handler, so we
    // can twiddle our thumbs
    while (1)
        tight_loop_contents();
```


What is sysclock frequency?  125 MHz?  this ain't no c64...

to get 44.1kHz i need a divider of?  2834.467120181

looking at some other example the wrap also affects this.

it's not clear yet.

but at 125Mhz

we can set a wrap of 254

and a divider of 12

and get 41010.498687664 Hz

close enough.

this example tweaks the sys clock to to get a nicer audio clock

https://github.com/rgrosset/pico-pwm-audio/blob/main/pico-pwm-audio.c

oh this line: `irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);`

well...

https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#interrupt-numbers


feck.

so, i assumed the pwm level would be 16 bit.

oh, is it tied to the wrap level?

anyway, send an 8bit value works.

my clock speed was correct too.

ah, better the rp2040 datasheet has more info


https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf

- interrupt request and DMA request on counter wrap

well that's handy....

wrap is a terrible name, it's the trigger level.

if i want to send 16bit samples i set wrap to 65535

oh, which gives me 1900 HERTZ!  bit slow...

yes that seems to be the case.  but the clock speed is no good

i can run at 12 bit and get about 31kHz

sounds correct too.

ok.

so for the reality situation

8 bit sound

a wrap of 255 or so

and in fact our original values are pretty good for 44.1kHz

we can probably drop to 22kHz

wrap:255

div:22

lands us at about 22.3kHz

as our sample is 44.1 we have to increment the index TWICE to make it about right.,

that is my hypothesis, let us try.

it is so.

ok, better read the rp2040 datasheet then...

Before looking at DMA I want to look at buffers.

I am thinking I need two buffers.  one is being streamed to the PWM while the other is filled.

when the buffer is empty i swap over.

is that normal? double buffering?

a search says "yes" this is sane.  the decision is mainly one of latency.

for DMA double bufferering appears to be necessary.

In BELA they use block based processing, which is buffers.

```cpp
// setup() runs once at the beginning of your program
bool setup(BelaContext *context, void *userData)
{
    // etc
}

// render() is called repeatedly by Bela for each audio block
void render(BelaContext *context, void *userData)
{
    // etc
}

// cleanup() runs once at the end of the program
void cleanup(BelaContext *context, void *userData)
{
}
```

the context provides several things, audio buffers, analog buffers, digital buffers, bidirectional too.

we care only for an audioOut buffer and samplesElapsed

also, I do not need to care about "frames" that is how many channels per sample do i need to fill.

it's one channel, mono so it's all 1.  hence just  samplesElapsed (u64)

userData is just some user struct that get passed around, so more user context data. i do not care for now.

how did I write my synth?  I made a `MonoWaveStream` that just fills a buffer when asked. and it get's it's next byte from `_voice.Output(Time).Out.Value`

`Time` then is `samplesElapsed`

so, trying to port my code...

I want a `Device` that has two buffers and 

- a pointer that tracks the current buffer 
- a read index

on the IRQ we just read another value from the buffer.

the device should also handle setting up sample rate but not bit depth as we can't really go beyond 8 bit

my code return `short` 16 bit samples. ok, deal with that later.

the Pico SDK is C/C++ so I can use that if need be.

how do i fill the buffer while continuing to service the irq?

threads? trigger another IRQ when swapping buffers?

skimming throught he examples, DMA offers control block chain - which is basically an array of buffers, or raise an IRQ when data is needed

there are timers, which use the system timers e.g. alarms

i feel like raising another IRQ when we need data is fine and sane.

there is a `pico_audio_pwm` sample in the `pico-extras`.  not making too much sense of it but:

it does some fancy things, like over-sampling and noise dithering (huh!?)

but how does it get it's data?

it defines audio buffer producer and consumer pools.

i can't follow the code but it looks like a DMA irq fires and asks for a new buffer....

i imagine the nice thing about using DMA+PIO is that a lot of the work that is NOT directly related to generating the next sample is offloaded to hardware and is very cycle exact etc.

but today that is beyond me.

SO.

## buffers and dma

Double buffer and use an IRQ to request more data.
do not care about DMA at this time.

fill the buffer so we can continuosly play a sine wave or something

ok. 

`Amplitude = sin(2*pi*f*t)`

and 

`t = samplesElapsed/sampleRate`

and sample rate as given in the rp2040 datasheet

`fsys = 125 MHz`

`fpwm = fsys/period`

`period = (WRAP+1)*(phase_correct+1)*(DIV+15/16)`

so we use wrap =254 and div =22 to get fpwm=21370.946198643

and that makes a noise that sounds fairly crap, lots of clicks and drift. 

lets add phase correction (halve the divider)

```
    pwm_config_set_clkdiv(&config, 11.f);
    pwm_config_set_wrap(&config, 254);
    pwm_config_set_phase_correct(&config, true);

```

not much better

hmm, wonder if it rentrant? make the buffers bigger

oh. that is an issue,  still clicks but much less.

BIGGER! 8k buffer!

actually it's a phase problem.  regular click.  

actually it's a bunch of bugs.  using `_samplesElapsed` which is still being incremented by the core0 while core1 tries to use it.

marshalling data to core1?  i think the FIFO is used? i just copy the value.  solid.

ok, reduce the buffer! 1k, just fine.

yeah buffer can be tiny, 32 bytes lol.

1 byte even.  ok, there's no real latency so far.

sweet as.

### PWM IRQ

From the docs:

"The same [PWM] pulse which sets the interrupt flag in INTR is also available as a one-cycle data request to the RP2040 system DMA. For each cycle the DMA sees a DREQ asserted, it will make one data transfer to its programmed location, in as timely a manner as possible. In combination with the double-buffered behaviour of CC and TOP, this allows the DMA to efficiently stream data to a PWM slice at a rate of one transfer per counter period. Alternatively, a PWM slice could serve as a pacing timer for DMA transfers to some other memory-mapped hardware."

DMA IRQ or DREQ seems like the go then.  next step.

Also: "the PWM block can also be used as a source of regular interrupt requests for non-PWM-related purposes"

Like polling MIDI or ADC...

Choice.

### DMA/DREQ

I'm struggling to see what benefit I would get here...

It seems the DMA will stream data to the PWM on each WRAP IRQ, which is good

Then a DMA IRQ will fire when it's buffer is empty?

we can then more or less immediately swap buffers and let DMA continue while then going on to fill the next buffer

which looks like it could all happen on core0

in this way the PWM IRQ work becomes completely offloaded to hardware

and the actual work of producing samples only needs to be checked for at the right time, when the buffer is empty

ok.  so maybe i can see the benefit.

This example is helpful, as it is basically what I want to do, ping-pong between two buffers.

https://github.com/raspberrypi/pico-examples/issues/112#issuecomment-1405075256

also

https://github.com/GregAC/pico-stuff/blob/main/pwm_dma/pwm_dma_interrupt_sequence.c

OK, let's give that a crack.

managed to bodge that together.  It is working but sounds quite bad - buzzy.

might look on the old oscilloscope.  A PICO SCOPE no less!

the old way, just the irq on the pwm produces some PWMish looking stuff
the new way produces a spike at 440Hz, click click click

i assume my config is not good.

i'll go read the pico-extras example of pwm audio.

well, I came across some info - samples MUST be 16 bit.  I do not understand the data sheet, but this works.



## core0 synth

when the time comes:

I am not confident to be able to port my C# code, it's all higher order functions and what not, and not straight forward at all to convert to C AFAIK.

But the maths is the same. so good enough as a reference.

- define a voice struct
DONE - waveforms
DONE - phase accumulation 
- trigger play new note
- envelope control
- filter
- 

oh my.

SO a sinewave, that was fine and dandy.
Noise, no problem.

Triangle?

blimey.



```c
double triangle(AudioContext_t *context, double frequency, double amplitude)
{
    // TODO: well this don't sound right...
    _phase += TWOPI * frequency / context->SampleRate;

    if (_phase >= TWOPI)
        _phase -= TWOPI;

    return (asin(sin(_phase)) / M_PI_2) * amplitude;
}
```

which is more or less the same as c#

```c#
        /// <summary>
        /// A triangle wave form.
        /// </summary>
        public static W Triangle() => (t, f, w, p) => Amplitude.Scale(Asin(Sin(Angle(f, t))) / HalfPI);

        public static double Angle(double t, double f) => TwoPI * f * t;

```

Time here is essentially SamplesElapsed/SampleRate.

In C# there is implicit conversion to double.  In C there is not.

The C code avoids this time calculation but has some odd phase discontinuity which I can't work out...

With a small buffer it sounded very odd, very distorted.
At a larger buffer size I hear the click as it jumps in phase.

I can printf out the contents of the buffers
and it is a nice looking triangle wave.

much confused

Square - OK. I have ignored pulsewidth for now.
Saw - OK.  

ok that's waveform now.

### trigger play note

hmmm.  

simply :

some player, that sets a frequency, sleeps for some time, sets a new frequency, etc loop

at this point we need some kind of context to represent state

let's add a Voice struct to the audio context.

Voice:
 frequency
 waveform

good enough for now.

```c
typedef struct Voice
{
    double frequency;
    enum waveform waveform;

} Voice_t;
```

```c
    while (true)
    {
        _context->Voice.frequency = 440;
        sleep_ms(200);
        _context->Voice.frequency = 660;
        sleep_ms(200);
        _context->Voice.frequency = 880;
        sleep_ms(200);
        _context->Voice.frequency = 660;
        sleep_ms(200);
    }
```

does it for me.

next up:

DONE - a frequency/note table
NAH DO IT LATER - multiple voices which will create an audioframe
NAH DO IT LATER - mix the voices

now i can test things a bit better
it sounds fairly crap lol :)

the waveforms have phase accumulation errors.  
and using sleep_ms instead of something tied to the sample rate makes... not much sense

OK.  So what seems to be happening is,

the waveform LOOKS good when i log it out and graph it.  it's perfect.

the code CRASHES when the buffer swaps ONLY on triangle. and the dma IRQ keeps on playing the same buffer over and over

hence the clicking...

why you crash?

to debug I need a SWD debugger - the best option is another pico, which I do not have to hand unless I rip apart something else, which I don't wanna do...

but knowing that it is actually crashing and not some phase problem as i thought may be enough to triage that bad boy

ok.  after some stuffing around it seems that arcsine is too slow and the buffers get corrupted.  the assertion is that arcsine is slower than 1/SampleRate seconds

so let's set the sample rate to 16384 or 8000 or something and see...

yep.  works* but sounds like crap.  ok. 

i wasn't expecting this kind of time budget.

hello stackoverflow:

https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm

```c
double acos(x) {
   return (-0.69813170079773212 * x * x - 0.87266462599716477) * x + M_PI_2;
}
```

this is fast enough and sounds OK.
scanning through the ARM M0+ datasheets looks like trig is not on the chip.
could consider a Lookup table instead i suppose.


ARM provides CMSIS-DSP optimised libraries.  
seems I have arm_math.h, does it have arccos or arcsin?

appears not.  but i do see a bunch of FIR filter functions.  looks interesting.

seems with the approximate arccos i can ran at 35kHz, so I have a little headroom if i am targeting 22kHz.
and I can always overclock.

DO i need to look at the emitted assembly?  Oh my C...

No issues in the C# version, but then again it was running on a processer that is 30+ times faster with trig intrinsics...

### Envelope

remove amplitude from Voice_t and let envelope control it

NoteOn - trigger attack-decay-sustain (onset)
NoteOff - trigger release



## core1 midi (or adc knob twiddling)


tbd

there is _TUSB_MIDI_DEVICE_H_  tinyusb midi device

add midi note number table
add midi command noteon, noteoff

midi events should be in a FIFO queue that is supplied with the current write buffer
so we can catch up on what and when
but tinyusb should be doing this i assume.


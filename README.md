# Raspberry Pi Pico "pico-synth"

Audio synthesizer running on a Raspberyy Pi Pico device.

Currently monophonic with TWO voices.

You WILL need to make a little circuit to drop the PWM voltage from 3.3 volts to about 1.1 volts and filter low/high noise out.  Or just risk it and plug in a speaker :)

Here is an example circuit: https://learn.adafruit.com/adding-basic-audio-ouput-to-raspberry-pi-zero/pi-zero-pwm-audio#low-pass-filter-wiring-1960165

I use `qjackctl` on linux to connect a midi keyboard via my PC.  I have not tested it directly with a MIDI controller directly attached (no PC involved), I assume it will work :)

You can find a `synth.uf2` file in the build folder which can be copied onto the Pico (hold bootsel while powering on connecting to a PC).

This is about the third C project I have written in the last thirty years, so you know... all bugs are my own.

## TinyUSB

pico-sdk ships an older version of tinyusb, so i have pulled latest of that module in order to get the newer usb host code.

Must be > v0.16

```
# update sdk
cd pico-sdk
git submodule update --init --recursive

# update tiny-usb
cd pico-sdk/lib/tinyusb
git pull origin master
```

midi host code from:  https://github.com/rppicomidi/usb_midi_host
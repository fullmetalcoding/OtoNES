# OtoNES
Why yet another Nintendo Emulator? Well, I worked on my own NES emulator back in high school, 20 some-odd years ago. 
It was quirky, just thrown together, and didn't really work all that well. Now, many years later and well into my career as a professional
software developer, I wanted to see if I could do a better job than teenage me with two decades of experience (not to mention all the new 
info on the NES hardware that has come out).


This project's dependencies are currently SDL2 and SDL2_TTF, however, the emulator core itself is meant to be graphics agnostic and doesn't
use any SDL calls in the PPU engine.

The 6502 emulator core is essentially gianlucag's mos6502 emulator (https://github.com/gianlucag/mos6502), however it has been extended
to support proper cycle timings, as the original mos6502 did not take these into account (and only allowed the user to run one instruction
at a time vice 1 actual clock cycle).


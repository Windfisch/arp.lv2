# arp.lv2

This is a simple LV2 MIDI processor to add [tracker-style arpeggiatos](https://youtu.be/OmeuhvYoij0?t=93)
to your favourite soft- or hardware synth.

Upon receiving a MIDI CC number 0x70, value 0xXY, pitchbend events will be emitted to shift the instrument by
0, then X, then Y, semitones, then falling back to 0. This emulates closely how trackers used to work.

In order to ensure correct tempo handling, you need to calibrate tempo initially by sending CC 0x70, value 0x00
twice, delayed by the desired step duration.

Sending MIDI clock events to the plugin is optional and will enable you to react to tempo changes without
recalibration.

Note that this plugin assumes pitchbend to bend exactly a full octave up and down.

## Building

Just type `make`. This will create the `arp.lv2/` directory, which you can move to `~/.lv2/`, so your LV2 host
can find it.

# pwmidicontrol

## Change volume using a MIDI device

Similar to [the python version](https://github.com/CZnavody19/Python-Midi-Translator) but written in C++ for PipeWire

Im using WirePlumber `wpctl` to control the volume

## How to use

1. Get the `main.cpp` file and install the RTMidi library for C++
2. Compile the source code using `g++ -o pwmidicontrol main.cpp -lrtmidi`
3. Set up the configuraton file `config.pwm` using the [example](config.pwm) and the `wpctl status` command
4. Execute with `./pwmidicontrol`

## Config file

There are three sections:
1. Sinks
2. Sources
3. Streams

The file is written in HTML-like fashion where:
- Each section begins with `<Section name>` and ends with `</Section name>`
- Each item in a section has a format like `<MIDI knob ID: audio sink/source/stream name>`

In the [example](config.pwm) knob 1 controls the volume of Firefox and knob 5 controls the volume of my sound card

## Disclaimer

This is a minimal working version and the code is kinda ugly

Also this is my first C++ project

Feel free to open an issue or PR

I plan on writing more features someday

## Features
- [x] Volume control
- [x] Get IDs at startup
- [x] Functional config file
- [ ] Refresh IDs at runtime
- [ ] Button presses (ie. pause/play, next, previous)
- [ ] Better logging and error handling
- [ ] Divide to more source files
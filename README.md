# pwmidicontrol

## Change volume using a MIDI device

Similar to [the python version](https://github.com/CZnavody19/Python-Midi-Translator) but written in C++ for PipeWire

Im using WirePlumber `wpctl` to control the volume

## How to use

1. Clone the repository and install the RTMidi library for C++, also install `cmake` and `make`
2. Create a `build` directory and cd into it
3. Run `cmake ..` and then compile the source code using `make`
4. Set up the configuraton file `config.pwm` using the [example](config.pwm) and the `wpctl status` command (the config file has to be one directory up of the executable)
5. Execute with `./pwmidicontrol`

## Config file

There are two sections:
1. Knobs
2. Buttons (not yet implemented)

The file is written in HTML-like fashion where:
- Each section begins with `<Section name>` and ends with `</Section name>`
- Each item in a section has a format like `<MIDI knob ID: Sink/Source/Stream; name>`

In the [example](config.pwm) knob 1 controls the volume of Firefox and knob 5 controls the volume of my sound card output

## Disclaimer

This is a better working version but the code is still kinda ugly

Also this is my first C++ project

Feel free to open an issue or PR

I plan on writing more features someday

## Features
- [x] Volume control
- [x] Get IDs at startup
- [x] Functional config file
- [ ] Refresh IDs and config at runtime (file change watch)
- [ ] Button presses (ie. pause/play, next, previous)
- [ ] Better logging and error handling
- [x] Divide to more source files
- [ ] Use a proper pipewire API (probably wont happen, im not that good lol)
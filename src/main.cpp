// pwmidicontrol - control PipeWire with a MIDI device
// Copyright (C) 2024  CZnavody19

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "midi.h"
#include "control.h"
#include "config.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <map>

void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    unsigned int nBytes = message->size();

    if (nBytes != 3)
    {
        return;
    }

    ControlType type = map_midi_type(message->at(0) & 0xF0);

    if (type == ControlType::UNKNOWN)
    {
        return;
    }

    int controller = message->at(1);

    std::tuple<std::string, DeviceType> entry = get_entry(controller, type);

    if (std::get<0>(entry) == "")
    {
        return;
    }

    set_volume((char *)std::get<0>(entry).c_str(), std::get<1>(entry), message->at(2));
}

int main()
{
    init_config();
    init_control();
    init_midi(midiCallback);

    // Keep program running to capture MIDI messages
    std::cout << "Listening for MIDI messages...\n";
    char input;
    std::cin >> input; // Press any key to exit

    destroy_midi();

    return 0;
}

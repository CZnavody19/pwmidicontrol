#pragma once

#include <structs.h>

#include <rtmidi/RtMidi.h>
#include <functional>

void init_midi(std::function<void(double, std::vector<unsigned char> *, void *)> callback);
void destroy_midi();

ControlType map_midi_type(int type);
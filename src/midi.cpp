#include "midi.h"

RtMidiIn *midiin;

ControlType map_midi_type(int type)
{
    switch (type)
    {
    case 0x90: // button on
        return ControlType::BUTTON;
    case 0xB0:
        return ControlType::KNOB;

    default:
        return ControlType::UNKNOWN;
    }
}

void init_midi(std::function<void(double, std::vector<unsigned char> *, void *)> callback)
{
    midiin = new RtMidiIn();
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 1) // There is a loopback interface
    {
        std::cout << "No ports available!\n";
        return;
    }

    // Open first available port that is not loopback
    midiin->openPort(1);

    if (midiin->isPortOpen())
    {
        std::cout << "Port is open!" << std::endl;
    }

    // Set callback for incoming MIDI messages
    midiin->setCallback([](double timeStamp, std::vector<unsigned char> *message, void *userData)
                        {
        auto& cb = *static_cast<std::function<void(double, std::vector<unsigned char> *, void *)>*>(userData);
        cb(timeStamp, message, userData); }, &callback);

    // Ignore sysex, timing, and active sensing messages
    midiin->ignoreTypes(true, true, true);
}

void destroy_midi()
{
    delete midiin;
}
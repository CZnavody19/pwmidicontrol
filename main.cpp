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

#include <rtmidi/RtMidi.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <map>

static std::map<std::string, std::string> configSinksMap;
static std::map<std::string, std::string> configSourcesMap;
static std::map<std::string, std::string> configStreamsMap;

static std::map<int, std::string> midiMap;

void get_entries(char *text, int begin, int end, std::map<std::string, std::string> *map)
{
    map->clear();
    char id[10], name[100];

    int lineStart = begin;
    while (lineStart < end)
    {
        // Find the next newline character
        int lineEnd = strchr(text + lineStart, '\n') - text;

        // Allocate memory for the line
        char *line = (char *)malloc(lineEnd - lineStart + 1); // +1 for null terminator

        // Copy the line into the char array
        strncpy(line, text + lineStart, lineEnd - lineStart);
        line[lineEnd - lineStart] = '\0'; // Add null terminator

        // Process the line
        if (sscanf(line, "<%[^:]: %[^\n>]>", id, name) == 2)
        {
            // Insert the parsed id and name into the map
            map->insert(std::make_pair(id, name));
        }

        // Free the allocated memory
        free(line);

        // Move to the next line
        lineStart = lineEnd + 1;
    }
}

void parse_config(char *config)
{
    int sinksBegin = strstr(config, "<Sinks>") - config;
    int sinksEnd = strstr(config, "</Sinks>") - config;

    int sourcesBegin = strstr(config, "<Sources>") - config;
    int sourcesEnd = strstr(config, "</Sources>") - config;

    int streamsBegin = strstr(config, "<Streams>") - config;
    int streamsEnd = strstr(config, "</Streams>") - config;

    get_entries(config, sinksBegin, sinksEnd, &configSinksMap);
    get_entries(config, sourcesBegin, sourcesEnd, &configSourcesMap);
    get_entries(config, streamsBegin, streamsEnd, &configStreamsMap);
}

void find_id(char *text, int begin, int end, char *name, char **id)
{
    *id = NULL;
    int lineStart = begin;
    while (lineStart < end)
    {
        // Find the next newline character
        int lineEnd = strchr(text + lineStart, '\n') - text;

        // Allocate memory for the line
        char *line = (char *)malloc(lineEnd - lineStart + 1); // +1 for null terminator

        // Copy the line into the char array
        strncpy(line, text + lineStart, lineEnd - lineStart);
        line[lineEnd - lineStart] = '\0'; // Add null terminator

        // Process the current line (in char array format)
        char *start = line;
        while (*start != '\0' && !isalnum(*start))
        {
            start++;
        }

        if (start != line)
        {
            memmove(line, start, strlen(start) + 1);
        }

        // Check if the line contains the name we are looking for
        if (strstr(line, name) != NULL)
        {
            int dot_position = strstr(line, ".") - line;

            *id = (char *)malloc(dot_position + 1);
            strncpy(*id, line, dot_position);
            (*id)[dot_position] = '\0'; // Null terminate the ID string
        }

        // Free the allocated memory
        free(line);

        // Move to the next line
        lineStart = lineEnd + 1;
    }
}

void extract_ids_and_names(char *output)
{
    // find all the sections
    int begin = strstr(output, "\nAudio") - output;
    int end = strstr(output, "\nVideo") - output;

    int sinksBegin = strstr(output + begin, " ├─ Sinks:") - output;
    int sinksEnd = strstr(output + begin, " ├─ Sink endpoints:") - output;

    int sourcesBegin = strstr(output + begin, " ├─ Sources:") - output;
    int sourcesEnd = strstr(output + begin, " ├─ Source endpoints:") - output;

    int streamsBegin = strstr(output + begin, " └─ Streams:") - output;

    for (auto &entry : configSinksMap)
    {
        char *id;
        find_id(output, sinksBegin, sinksEnd, (char *)entry.second.c_str(), &id);
        if (id != NULL)
        {
            // Insert the ID into the MIDI map
            midiMap.insert(std::make_pair(stoi(entry.first), id));

            free(id);
        }
    }

    for (auto &entry : configSourcesMap)
    {
        char *id;
        find_id(output, sourcesBegin, sourcesEnd, (char *)entry.second.c_str(), &id);
        if (id != NULL)
        {
            // Insert the ID into the MIDI map
            midiMap.insert(std::make_pair(stoi(entry.first), id));

            free(id);
        }
    }

    for (auto &entry : configStreamsMap)
    {
        char *id;
        find_id(output, streamsBegin, end, (char *)entry.second.c_str(), &id);
        if (id != NULL)
        {
            // Insert the ID into the MIDI map
            midiMap.insert(std::make_pair(stoi(entry.first), id));

            free(id);
        }
    }
}

void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    unsigned int nBytes = message->size();
    for (unsigned int i = 0; i < nBytes; i++)
    {
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    }
    std::cout << std::endl;

    if (nBytes > 0 && (message->at(0) & 0xF0) == 0xB0)
    { // Control Change message
        int controller = message->at(1);
        int value = message->at(2);

        value = (value * 100) / 127; // Convert to percentage

        if (midiMap.find(controller) == midiMap.end())
        {
            return;
        }

        char *id = (char *)midiMap[controller].c_str();

        // Adjust the volume of the sink/source/stream with the given ID
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "wpctl set-volume %s %d%%", id, value);
        system(cmd);
    }
}

int main()
{
    RtMidiIn *midiin = new RtMidiIn();

    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 1) // There is a loopback interface
    {
        std::cout << "No ports available!\n";
        return 1;
    }

    // Open first available port that is not loopback
    midiin->openPort(1);

    if (midiin->isPortOpen())
    {
        std::cout << "Port is open!" << std::endl;
    }

    // Open the config file
    FILE *fp = fopen("config.pwm", "r");
    if (fp == NULL)
    {
        perror("Failed to run command");
        return EXIT_FAILURE;
    }

    // copy the file contents to a char array
    char *config = NULL;
    size_t totalConfigSize = 0;

    static char configBuff[1024];
    size_t configN;
    while ((configN = fread(configBuff, 1, sizeof(configBuff) - 1, fp)) > 0)
    {
        totalConfigSize += configN;
        config = (char *)realloc(config, totalConfigSize);
        memcpy(config + (totalConfigSize - configN), configBuff, configN);
    }

    // Close the pipe
    pclose(fp);

    // Parse the config file
    parse_config(config);

    free(config);

    // Open the wpctl status output using popen to capture the command output
    FILE *cp = popen("wpctl status", "r");
    if (cp == NULL)
    {
        perror("Failed to run command");
        return EXIT_FAILURE;
    }

    // copy the output of the command to a char array
    char *statusOut = NULL;
    size_t totalStatusSize = 0;

    static char statusBuff[1024];
    size_t statusN;
    while ((statusN = fread(statusBuff, 1, sizeof(statusBuff) - 1, cp)) > 0)
    {
        totalStatusSize += statusN;
        statusOut = (char *)realloc(statusOut, totalStatusSize);
        memcpy(statusOut + (totalStatusSize - statusN), statusBuff, statusN);
    }

    // Close the pipe
    pclose(cp);

    // Extract and print the IDs and names
    extract_ids_and_names(statusOut);

    free(statusOut);

    // Set callback for incoming MIDI messages
    midiin->setCallback(&midiCallback);

    // Ignore sysex, timing, and active sensing messages
    midiin->ignoreTypes(true, true, true);

    // Keep program running to capture MIDI messages
    std::cout << "Listening for MIDI messages...\n";
    char input;
    std::cin >> input; // Press any key to exit

    delete midiin;
    return 0;
}
#include "config.h"

// id: (type, name)
std::map<int, std::tuple<std::string, std::string>> knobMap;
std::map<int, std::tuple<std::string, std::string>> buttonMap;

void get_entries(char *text, int begin, int end, std::map<int, std::tuple<std::string, std::string>> *map)
{
    map->clear();
    char id[10], type[30], name[100];

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
        if (sscanf(line, "<%[^:]: %[^;]; %[^\n>]>", id, type, name) == 3)
        {
            int Id = atoi(id);
            // Insert the parsed id and name into the map
            map->insert(std::make_pair(Id, std::make_tuple(type, name)));
        }

        // Free the allocated memory
        free(line);

        // Move to the next line
        lineStart = lineEnd + 1;
    }
}

void refresh_config()
{
    // Open the config file
    FILE *fp = fopen("../config.pwm", "r");
    if (fp == NULL)
    {
        perror("Failed to run command");
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
    int knobsBegin = strstr(config, "<Knobs>") - config;
    int knobsEnd = strstr(config, "</Knobs>") - config;

    int buttonsBegin = strstr(config, "<Buttons>") - config;
    int buttonsEnd = strstr(config, "</Buttons>") - config;

    get_entries(config, knobsBegin, knobsEnd, &knobMap);
    get_entries(config, buttonsBegin, buttonsEnd, &buttonMap);

    // Free the allocated memory
    free(config);
}

void init_config()
{
    refresh_config();

    std::cout << "Knobs:" << std::endl;
    for (auto const &entry : knobMap)
    {
        std::cout << entry.first << " " << std::get<0>(entry.second) << " " << std::get<1>(entry.second) << std::endl;
    }

    std::cout << "Buttons:" << std::endl;
    for (auto const &entry : buttonMap)
    {
        std::cout << entry.first << " " << std::get<0>(entry.second) << " " << std::get<1>(entry.second) << std::endl;
    }
}

std::tuple<std::string, DeviceType> get_entry(int id, ControlType type)
{
    std::tuple<std::string, std::string> entry;
    switch (type)
    {
    case ControlType::KNOB:
        entry = knobMap[id];
        break;
    case ControlType::BUTTON:
        entry = buttonMap[id];
        break;
    }

    if (std::get<0>(entry) == "")
    {
        return std::make_tuple("", DeviceType::UNKNOWN_DEVICE);
    }

    static const std::unordered_map<std::string, DeviceType> stringToEnum = {
        {"sink", SINK},
        {"source", SOURCE},
        {"stream", STREAM}};

    std::transform(std::get<0>(entry).begin(), std::get<0>(entry).end(), std::get<0>(entry).begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    return std::make_tuple(std::get<1>(entry), stringToEnum.at(std::get<0>(entry)));
}
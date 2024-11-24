#include "control.h"

std::map<std::string, int> sinkIdMap;
std::map<std::string, int> sourceIdMap;
std::map<std::string, int> streamIdMap;

bool is_empty_or_whitespace(const char *line)
{
    while (*line != '\0')
    {
        if (!isspace(*line))
        {
            return false;
        }
        line++;
    }
    return true;
}

void extract_ids(char *input)
{
    int begin = strstr(input, "\nAudio") - input;
    int end = strstr(input, "\nVideo") - input;

    int sinksBegin = strstr(input + begin, " ├─ Sinks:") - input;
    int sourcesBegin = strstr(input + begin, " ├─ Sources:") - input;
    int streamsBegin = strstr(input + begin, " └─ Streams:") - input;

    int current = sinksBegin;
    int currentMode = 0;

    while (current < end)
    {
        // Find the next newline character
        int lineEnd = strchr(input + current, '\n') - input;

        // Allocate memory for the line
        char *line = (char *)malloc(lineEnd - current + 1); // +1 for null terminator

        // Copy the line into the char array
        strncpy(line, input + current, lineEnd - current);
        line[lineEnd - current] = '\0'; // Add null terminator

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

        // determine the mode

        if (strstr(line, "Sinks:") != NULL)
        {
            currentMode = 0;
        }
        else if (strstr(line, "Sources:") != NULL)
        {
            currentMode = 1;
        }
        else if (strstr(line, "Streams:") != NULL)
        {
            currentMode = 2;
        }

        if (is_empty_or_whitespace(line))
        {
            // Free the allocated memory
            free(line);

            // Move to the next line
            current = lineEnd + 1;
            continue;
        }

        // if line doesnt begin with a number, skip it
        if (!isdigit(line[0]))
        {
            // Free the allocated memory
            free(line);

            // Move to the next line
            current = lineEnd + 1;
            continue;
        }

        char *id;
        char *name;

        int dot_position = strstr(line, ".") - line;

        id = (char *)malloc(dot_position + 1);
        strncpy(id, line, dot_position);
        id[dot_position] = '\0'; // Null terminate the ID string

        name = (char *)malloc(strlen(line) - dot_position);
        strncpy(name, line + dot_position + 2, strlen(line) - dot_position - 1);
        name[strlen(line) - dot_position] = '\0'; // Null terminate the name string

        switch (currentMode)
        {
        case 0:
            sinkIdMap.insert(std::make_pair(name, atoi(id)));
            break;

        case 1:
            sourceIdMap.insert(std::make_pair(name, atoi(id)));
            break;

        case 2:
            streamIdMap.insert(std::make_pair(name, atoi(id)));
            break;
        }

        // Free the allocated memory
        free(line);

        // Move to the next line
        current = lineEnd + 1;
    }
}

void refresh_devices()
{
    // Open the wpctl status output using popen to capture the command output
    FILE *cp = popen("wpctl status", "r");
    if (cp == NULL)
    {
        perror("Failed to run command wpctl status");
        return;
    }

    // copy the output of the command to a char array
    char *statusOut = NULL;
    size_t totalStatusSize = 0;

    static char statusBuff[1024];
    size_t statusN;
    while ((statusN = fread(statusBuff, 1, sizeof(statusBuff) - 1, cp)) > 0)
    {
        totalStatusSize += statusN;
        char *temp = (char *)realloc(statusOut, totalStatusSize + 1); // +1 for null-terminator
        if (temp == NULL)
        {
            free(statusOut);
            pclose(cp);
            return;
        }
        statusOut = temp;
        memcpy(statusOut + (totalStatusSize - statusN), statusBuff, statusN);
        statusOut[totalStatusSize] = '\0'; // Null-terminate the buffer
    }

    // Close the pipe
    pclose(cp);

    // Extract and print the IDs and names
    if (statusOut != NULL)
    {
        sinkIdMap.clear();
        sourceIdMap.clear();
        streamIdMap.clear();

        extract_ids(statusOut);
        free(statusOut);
    }
}

void init_control()
{
    refresh_devices();
}

void set_volume(char *name, DeviceType type, int value)
{
    value = (value * 100) / 127; // Convert to percentage

    int id;
    switch (type)
    {
    case DeviceType::SINK:
        for (auto const &entry : sinkIdMap)
        {
            if (entry.first.find(name) != std::string::npos)
            {
                id = entry.second;
                break;
            }
        }
        break;

    case DeviceType::SOURCE:
        for (auto const &entry : sourceIdMap)
        {
            if (entry.first.find(name) != std::string::npos)
            {
                id = entry.second;
                break;
            }
        }
        break;

    case DeviceType::STREAM:
        for (auto const &entry : streamIdMap)
        {
            if (entry.first.find(name) != std::string::npos)
            {
                id = entry.second;
                break;
            }
        }
        break;

    default:
        return;
    }

    // Adjust the volume of the sink/source/stream with the given ID
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "wpctl set-volume %d %d%%", id, value);
    system(cmd);
}

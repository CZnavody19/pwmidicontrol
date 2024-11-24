#pragma once

#include "structs.h"

#include <map>
#include <cstring>
#include <string>

using namespace std::__cxx11; // for stoi

void init_control();

void set_volume(char *name, DeviceType type, int value);
void refresh_devices();
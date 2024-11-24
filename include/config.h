#pragma once

#include "structs.h"

#include <tuple>
#include <string>
#include <cstring>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iostream>

void init_config();

std::tuple<std::string, DeviceType> get_entry(int id, ControlType type);
void refresh_config();
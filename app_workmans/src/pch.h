#pragma once
#include <algorithm>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <fstream>
#include <optional>
#include <chrono>
#include <set>
#include <cstring>

// 92,967 lines. scene.h pulls it in, so it was re-parsed for every .cpp that includes this.
//    Putting it in the PCH digs through it once per project.
#include "entt.hpp"

#include "core/log.h"
#include "debugs/instrumentation.h"

#ifdef APP_WMS_PLATFORM_WINDOWS
#include <Windows.h>
#endif
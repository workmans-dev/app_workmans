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

// 🟢 92,967줄. scene.h가 끌어오므로 이걸 포함하는 .cpp마다 매번 다시 파싱됐다.
//    PCH에 넣으면 프로젝트당 한 번만 판다.
#include "entt.hpp"

#include "core/log.h"
#include "debugs/instrumentation.h"

#ifdef APP_WMS_PLATFORM_WINDOWS
#include <Windows.h>
#endif
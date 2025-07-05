#include "cpu_backend.h"
#include "arm_translator.h"
#include <chrono>
#include <cstring>
#include <cstdio>

// Remove all SwitchCPUBackend function definitions that are also defined in switch_cpu_backend.cpp
// Only keep unique/shared logic if any.

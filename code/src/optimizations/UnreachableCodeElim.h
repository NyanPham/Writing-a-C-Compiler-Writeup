#pragma once

#include <variant>
#include "../CFG.h"

// Remove unreachable code and useless jumps/labels/blocks from a TACKY CFG.
cfg::Graph<std::monostate> eliminateUnreachableCode(const cfg::Graph<std::monostate> &cfg, bool debug);

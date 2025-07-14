#pragma once

#include <set>
#include <string>
#include <vector>
#include <memory>
#include "../Tacky.h"

// Returns the set of variable names whose address is taken in the given TACKY instruction list.
std::set<std::string> analyzeAddressTaken(const std::vector<std::shared_ptr<TACKY::Instruction>> &instructions);
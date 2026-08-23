#pragma once

#include <memory>
#include <string>
#include <vector>

#include "drcheck/rules/Rule.h"

namespace drcheck::io {

class TclRuleParser
{
public:
    static std::vector<std::unique_ptr<rules::Rule>> load(const std::string& filePath);
};

}
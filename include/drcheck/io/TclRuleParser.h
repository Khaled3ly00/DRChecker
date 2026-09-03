#pragma once

#include <memory>
#include <string>
#include <vector>

#include "drcheck/rules/Rule.h"
#include "drcheck/domain/LayerRegistry.h"

namespace drcheck::io {

class TclRuleParser
{
public:
    static std::vector<std::unique_ptr<rules::Rule>> load(const std::string& filePath, domain::LayerRegistry& registry);
};

}
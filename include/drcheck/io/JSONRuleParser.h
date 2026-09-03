#pragma once

#include "drcheck/rules/Rule.h"
#include "drcheck/domain/LayerRegistry.h"

#include <string>
#include <memory>

namespace drcheck::io {
class JSONRuleParser
{
public:
    static std::vector<std::unique_ptr<rules::Rule>> load(const std::string& filePath, domain::LayerRegistry& layerRegistry);
};
}
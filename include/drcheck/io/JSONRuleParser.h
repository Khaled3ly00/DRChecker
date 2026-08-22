#pragma once

#include "drcheck/rules/Rule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/DensityRule.h"

#include <nlohmann/json.hpp>
#include <string>
#include <memory>

namespace drcheck::io {
class JSONRuleParser
{
public:
    std::vector<std::unique_ptr<rules::Rule>> load(const std::string& filePath) const;

private:
    static rules::DensityLimit limitFromString(const std::string& limit);
    static geometry::BoundingBox parseBoundingBox(const nlohmann::json& json);

};
}
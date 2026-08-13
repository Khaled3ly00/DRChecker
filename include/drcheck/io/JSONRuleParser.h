#pragma once

#include "drcheck/rules/Rule.h"
#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinEnclosureRule.h"

#include <string>
#include <memory>

namespace drcheck::io {
class JSONRuleParser
{
public:
    std::vector<std::unique_ptr<rules::Rule>> load(const std::string& filePath) const;
};
}
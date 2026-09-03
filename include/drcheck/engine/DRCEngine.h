#pragma once

#include <memory>
#include <vector>

#include "drcheck/domain/Shape.h"
#include "drcheck/domain/Violation.h"
#include "drcheck/rules/Rule.h"

namespace drcheck::engine {

class DRCEngine
{
public:
    static std::vector<domain::Violation> run (const std::vector<domain::Shape>& shapes, const std::vector<std::unique_ptr<rules::Rule>>& rules);
};

}
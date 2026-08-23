#pragma once

#include <optional>
#include <string>
#include <vector>

#include "drcheck/domain/Violation.h"

namespace drcheck::engine {

struct DRCRunConfig
{
    std::string layoutPath;
    std::string rulesPath;
    std::string reportPath;
    std::optional<std::string> svgPath;
};

class DRCRunner
{
public:
    static std::vector<domain::Violation> run(const DRCRunConfig& config);
};

}
#pragma once

#include <optional>
#include <string>
#include <vector>
#include <memory>

#include "drcheck/domain/LayerRegistry.h"
#include "drcheck/domain/Shape.h"
#include "drcheck/domain/Violation.h"

namespace drcheck::engine {

struct DRCRunResult
{
    std::unique_ptr<domain::LayerRegistry> layerRegistry;
    std::vector<domain::Shape> shapes;
    std::vector<domain::Violation> violations;
};

struct DRCRunConfig
{
    std::string layoutPath;
    std::string rulesPath;
    std::string reportPath;
    std::optional<std::string> svgPath;
	std::optional<std::string> topCellName;
};

class DRCRunner
{
public:
    static DRCRunResult run(const DRCRunConfig& config);
};

}
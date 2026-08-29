#include "drcheck/engine/DRCRunner.h"
#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/io/SVGReportWriter.h"
#include "drcheck/io/TclRuleParser.h"
#include "drcheck/layout/LayoutNormalizer.h"

#include <filesystem>

namespace drcheck::engine {

std::vector<domain::Violation> DRCRunner::run(const DRCRunConfig& config)
{
    const auto rawShapes = io::JSONLayoutParser::load(config.layoutPath);

    const auto shapes = layout::LayoutNormalizer::normalize(rawShapes);

    std::vector<std::unique_ptr<rules::Rule>> rules;

    const std::string extension = std::filesystem::path(config.rulesPath).extension().string();

    if (extension == ".json")
    {
        rules = io::JSONRuleParser::load(config.rulesPath);
    }
    else if (extension == ".tcl")
    {
        rules = io::TclRuleParser::load(config.rulesPath);
    }
    else
    {
        throw std::invalid_argument("Unsupported rule file format: " + extension);
    }

    const auto violations = DRCEngine::run(shapes, rules);

    io::JSONReportWriter::write(violations, config.reportPath);

    if (config.svgPath.has_value())
    {
        io::SVGReportWriter::write(shapes, violations, config.svgPath.value());
    }

    return violations;
}

}
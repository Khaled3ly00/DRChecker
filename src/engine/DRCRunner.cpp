#include "drcheck/engine/DRCRunner.h"
#include "drcheck/engine/DRCEngine.h"
#include "drcheck/io/JSONLayoutParser.h"
#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/io/JSONReportWriter.h"
#include "drcheck/io/SVGReportWriter.h"
#include "drcheck/io/TclRuleParser.h"
#include "drcheck/layout/LayoutNormalizer.h"
#include "drcheck/domain/LayerRegistry.h"
#include "drcheck/io/GDSLayoutParser.h"

#include <filesystem>

namespace drcheck::engine {

std::vector<domain::Violation> DRCRunner::run(const DRCRunConfig& config)
{
    domain::LayerRegistry layerRegistry;

    std::vector<std::unique_ptr<rules::Rule>> rules;

    const std::string rulesExtension = std::filesystem::path(config.rulesPath).extension().string();

    if (rulesExtension == ".json")
    {
        rules = io::JSONRuleParser::load(config.rulesPath, layerRegistry);
    }
    else if (rulesExtension == ".tcl")
    {
        rules = io::TclRuleParser::load(config.rulesPath, layerRegistry);
    }
    else
    {
        throw std::invalid_argument("Unsupported rule file format: " + rulesExtension);
    }

    std::vector<domain::Shape> rawShapes;

    const std::string layoutExtension = std::filesystem::path(config.layoutPath).extension().string();

    if (layoutExtension == ".gds" || layoutExtension == ".gdsii")
    {
        rawShapes = io::GDSLayoutParser::load(config.layoutPath, layerRegistry, config.topCellName);
    }
    else if (layoutExtension == ".json")
    {
        if (config.topCellName.has_value())
        {
            throw std::invalid_argument("Top-level cell selection is only supported for GDSII layouts");
        }

        rawShapes = io::JSONLayoutParser::load(config.layoutPath, layerRegistry);
    } 
    else
    {
        throw std::invalid_argument("Unsupported layout file format: " + layoutExtension);
    }

    const auto shapes = layout::LayoutNormalizer::normalize(rawShapes);

    const auto violations = DRCEngine::run(shapes, rules);

    io::JSONReportWriter::write(violations, config.reportPath);

    if (config.svgPath.has_value())
    {
        io::SVGReportWriter::write(shapes, violations, config.svgPath.value());
    }

    return violations;
}

}
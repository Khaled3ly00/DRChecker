#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/rules/RuleFactory.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/rules/DensityRule.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace {

drcheck::rules::DensityLimit limitFromString(const std::string& limit)
{
    if (limit == "Minimum")
    {
        return drcheck::rules::DensityLimit::Minimum;
    }

    if (limit == "Maximum")
    {
        return drcheck::rules::DensityLimit::Maximum;
    }

    throw std::invalid_argument("Unknown density limit: " + limit);
}
drcheck::geometry::BoundingBox parseBoundingBox(const nlohmann::json& json)
{
    return drcheck::geometry::BoundingBox(
        json.at("minX").get<double>(),
        json.at("minY").get<double>(),
        json.at("maxX").get<double>(),
        json.at("maxY").get<double>()
    );
}

}
namespace drcheck::io {
    std::vector<std::unique_ptr<rules::Rule>> JSONRuleParser::load(const std::string& filePath) {
        std::ifstream input(filePath);

        if (!input) {
            throw std::runtime_error("Unable to open rules file: " + filePath);
        }

        nlohmann::json json;
        input >> json;

        // Check if JSON contains an array "rules"
        if (!json.contains("rules") || !json["rules"].is_array())
        {
            throw std::invalid_argument("Rules file must contain a rules array");
        }
        // Create reserved vector of Pointers of type Rule
        std::vector<std::unique_ptr<rules::Rule>> parsedRules;
        parsedRules.reserve(json["rules"].size());

        for (const auto& ruleJson : json["rules"])
        {
            // Rule must contain a JSON object not string, int...
            if (!ruleJson.is_object()) {
                throw std::invalid_argument("Each rule must be a JSON object");
            }

            const std::string type = ruleJson.at("type").get<std::string>();

            rules::RuleParameters params;

            if (type == "MinWidth")
            {

                params.layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                params.value = ruleJson.at("value").get<double>();

                parsedRules.push_back(rules::RuleFactory::create("min_width", params));
            }
            else if (type == "MinSpacing")
            {
                params.layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                params.value = ruleJson.at("value").get<double>();

                parsedRules.push_back(rules::RuleFactory::create("min_spacing", params));
            }
            else if (type == "MinEnclosure")
            {
                params.innerLayer = domain::layerFromString(ruleJson.at("innerLayer").get<std::string>());
                params.outerLayer = domain::layerFromString(ruleJson.at("outerLayer").get<std::string>());
                params.value = ruleJson.at("value").get<double>();

                parsedRules.push_back(rules::RuleFactory::create("min_enclosure", params));
            }
            else if (type == "Density")
            {
                params.layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                params.densityLimit = limitFromString(ruleJson.at("limit").get<std::string>());
                params.value = ruleJson.at("value").get<double>();
                params.windowSize = ruleJson.at("windowSize").get<double>();
                params.windowStep = ruleJson.at("windowStep").get<double>();

                if (ruleJson.contains("analysisWindow"))
                {
                    params.analysisWindow = parseBoundingBox(ruleJson.at("analysisWindow"));
                }

                parsedRules.push_back(rules::RuleFactory::create("density", params));
            }
            else
            {
                throw std::invalid_argument("Unknown rule type: " + type);
            }
        }
        return parsedRules;
    }
}
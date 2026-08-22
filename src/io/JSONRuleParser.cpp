#include "drcheck/io/JSONRuleParser.h"

#include <fstream>
#include <stdexcept>


namespace drcheck::io {
    std::vector<std::unique_ptr<rules::Rule>> JSONRuleParser::load(const std::string& filePath) const {
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

            if (type == "MinWidth")
            {
                const domain::Layer layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                const double value = ruleJson.at("value").get<double>();

                parsedRules.push_back(std::make_unique<rules::MinWidthRule>(layer, value));
            }
            else if (type == "MinSpacing")
            {
                const domain::Layer layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                const double value = ruleJson.at("value").get<double>();

                parsedRules.push_back(std::make_unique<rules::MinSpacingRule>(layer, value));
            }
            else if (type == "MinEnclosure")
            {
                const domain::Layer innerLayer = domain::layerFromString(ruleJson.at("innerLayer").get<std::string>());
                const domain::Layer outerLayer = domain::layerFromString(ruleJson.at("outerLayer").get<std::string>());
                const double value = ruleJson.at("value").get<double>();

                parsedRules.push_back(std::make_unique<rules::MinEnclosureRule>(innerLayer, outerLayer, value));
            }
            else if (type == "Density")
            {
                const domain::Layer layer = domain::layerFromString(ruleJson.at("layer").get<std::string>());
                const rules::DensityLimit limit = limitFromString(ruleJson.at("limit").get<std::string>());
                const double requiredDensity = ruleJson.at("requiredDensity").get<double>();
                const double windowSize = ruleJson.at("windowSize").get<double>();
                const double windowStep = ruleJson.at("windowStep").get<double>();
                if (ruleJson.contains("analysisWindow"))
                {
                    const geometry::BoundingBox analysisWindow = parseBoundingBox(ruleJson.at("analysisWindow"));

                    parsedRules.push_back(std::make_unique<rules::DensityRule>(layer, limit, requiredDensity, windowSize, windowStep, analysisWindow));
                }
                else
                {
                    parsedRules.push_back(std::make_unique<rules::DensityRule>(layer, limit, requiredDensity, windowSize, windowStep));
                }
            }
            else
            {
                throw std::invalid_argument("Unknown rule type: " + type);
            }
        }
        return parsedRules;
    }

    rules::DensityLimit JSONRuleParser::limitFromString(const std::string& limit)
    {
        if (limit == "Minimum")
        {
            return rules::DensityLimit::Minimum;
        }

        if (limit == "Maximum")
        {
            return rules::DensityLimit::Maximum;
        }

        throw std::invalid_argument("Unknown density limit: " + limit);
    }

    geometry::BoundingBox JSONRuleParser::parseBoundingBox(const nlohmann::json& json)
    {
        return geometry::BoundingBox(
            json.at("minX").get<double>(),
            json.at("minY").get<double>(),
            json.at("maxX").get<double>(),
            json.at("maxY").get<double>()
        );
    }
}
#include "drcheck/io/JSONRuleParser.h"

#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

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
            else
            {
                throw std::invalid_argument("Unknown rule type: " + type);
            }
        }
        return parsedRules;
    }
}
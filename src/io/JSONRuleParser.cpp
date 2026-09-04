#include "drcheck/io/JSONRuleParser.h"
#include "drcheck/rules/RuleFactory.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/rules/DensityRule.h"
#include "drcheck/rules/MinEnclosureRule.h"

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
    std::vector<std::unique_ptr<rules::Rule>> JSONRuleParser::load(const std::string& filePath, domain::LayerRegistry& layerRegistry) {
        std::ifstream input(filePath);

        if (!input) {
            throw std::runtime_error("Unable to open rules file: " + filePath);
        }

        nlohmann::json json;
        input >> json;

        if (!json.is_object())
        {
            throw std::invalid_argument("Rules file must contain a JSON object");
        }

        if (!json.contains("layers") || !json["layers"].is_array())
        {
            throw std::invalid_argument("Rules file must contain a layers array");
        }

        for (const auto& layerJson : json["layers"])
        {
            if (!layerJson.is_object())
            {
                throw std::invalid_argument("Each layer declaration must be a JSON object");
            }

            if (!layerJson.contains("name")) {
                throw std::invalid_argument("Each layer must contain a name");
            }

            if (!layerJson.contains("gdsMappings") || !layerJson["gdsMappings"].is_array() || layerJson["gdsMappings"].empty())
            {
                throw std::invalid_argument("Each layer must contain a non-empty gdsMappings array");
            }

            const std::string name = layerJson.at("name").get<std::string>();

            const auto& mappings = layerJson.at("gdsMappings");

            for (const auto& mappingJson : mappings)
            {
                if (!mappingJson.is_object())
                {
                    throw std::invalid_argument("Each GDS mapping must be a JSON object");
                }
                if (!mappingJson.contains("layer") || !mappingJson["layer"].is_number_integer())
                {
                    throw std::invalid_argument("Each GDS mapping must contain an integer layer");
                }

                if (!mappingJson.contains("datatype") || !mappingJson["datatype"].is_number_integer())
                {
                    throw std::invalid_argument("Each GDS mapping must contain an integer datatype");
                }
            }

            // Only declare layer after all validations pass
            const domain::Layer* layer = layerRegistry.declare(name);

            for (const auto& mappingJson : mappings)
            {
                const int gdsLayer = mappingJson.at("layer").get<int>();
                const int datatype = mappingJson.at("datatype").get<int>();

                layerRegistry.mapGDS(layer, gdsLayer, datatype);
            }
        }

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

                params.layer = layerRegistry.resolve(ruleJson.at("layer").get<std::string>());
                params.value = ruleJson.at("value").get<double>();

                parsedRules.push_back(rules::RuleFactory::create("min_width", params));
            }
            else if (type == "MinSpacing")
            {
                params.layer = layerRegistry.resolve(ruleJson.at("layer").get<std::string>());
                params.value = ruleJson.at("value").get<double>();

                parsedRules.push_back(rules::RuleFactory::create("min_spacing", params));
            }
            else if (type == "MinEnclosure")
            {
                params.innerLayer = layerRegistry.resolve(ruleJson.at("innerLayer").get<std::string>());

                if (ruleJson.contains("enclosureOptions"))
                {
                    const auto& optionsJson = ruleJson.at("enclosureOptions");

                    if (!optionsJson.is_array() || optionsJson.empty())
                    {
                        throw std::invalid_argument("MinEnclosure enclosureOptions must be a non-empty array");
                    }

                    std::vector<rules::EnclosureOption> enclosureOptions;
                    enclosureOptions.reserve(optionsJson.size());

                    for (const auto& optionJson : optionsJson)
                    {
                        const domain::Layer* outerLayer = layerRegistry.resolve(optionJson.at("outerLayer").get<std::string>());
                        const double allSidesMinEnclosure = optionJson.at("allSidesMinEnclosure").get<double>();
                        const bool hasFirstPair = optionJson.contains("firstPairMinEnclosure");
                        const bool hasSecondPair = optionJson.contains("secondPairMinEnclosure");

                        if (hasFirstPair != hasSecondPair)
                        {
                            throw std::invalid_argument("MinEnclosure option must provide both firstPairMinEnclosure and secondPairMinEnclosure"
                            );
                        }

                        if (hasFirstPair)
                        {
                            enclosureOptions.emplace_back(outerLayer, allSidesMinEnclosure, optionJson.at("firstPairMinEnclosure").get<double>(),
                                optionJson.at("secondPairMinEnclosure").get<double>());
                        }
                        else
                        {
                            enclosureOptions.emplace_back(outerLayer, allSidesMinEnclosure);
                        }
                    }

                    params.enclosureOptions = std::move(enclosureOptions);
                }
                else
                {
                    // Backward-compatible MinEnclosure format
                    params.outerLayer = layerRegistry.resolve(ruleJson.at("outerLayer").get<std::string>());
                    params.value = ruleJson.at("value").get<double>();
                }

                parsedRules.push_back(rules::RuleFactory::create("min_enclosure", params));
            }
            else if (type == "Density")
            {
                params.layer = layerRegistry.resolve(ruleJson.at("layer").get<std::string>());
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
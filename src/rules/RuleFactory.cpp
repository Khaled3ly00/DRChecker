#include "drcheck/rules/RuleFactory.h"

#include "drcheck/rules/MinWidthRule.h"
#include "drcheck/rules/MinSpacingRule.h"
#include "drcheck/rules/MinEnclosureRule.h"

#include <stdexcept>

namespace drcheck::rules {

    std::unique_ptr<Rule> RuleFactory::create(const std::string& type, const RuleParameters& params)
    {
        if (type == "min_width")
        {
            if (!params.layer.has_value() || !params.value.has_value())
            {
                throw std::invalid_argument("min_width requires layer and value");
            }

            return std::make_unique<MinWidthRule>(params.layer.value(), params.value.value());
        }

        if (type == "min_spacing")
        {
            if (!params.layer.has_value() || !params.value.has_value())
            {
                throw std::invalid_argument("min_spacing requires layer and value");
            }

            return std::make_unique<MinSpacingRule>(params.layer.value(), params.value.value());
        }

        if (type == "min_enclosure")
        {
            if (!params.innerLayer.has_value())
            {
                throw std::invalid_argument("MinEnclosureRule requires innerLayer");
            }

            if (params.enclosureOptions.has_value())
            {
                return std::make_unique<MinEnclosureRule>(params.innerLayer.value(), params.enclosureOptions.value());
            }
            if (!params.outerLayer.has_value() || !params.value.has_value())
            {
                throw std::invalid_argument("MinEnclosureRule requires enclosure options or outerLayer and value");
            }

            return std::make_unique<MinEnclosureRule>(params.innerLayer.value(), params.outerLayer.value(), params.value.value());
        }

        if (type == "density")
        {
            if (!params.layer.has_value() ||
                !params.value.has_value() ||
                !params.densityLimit.has_value() ||
                !params.windowSize.has_value() ||
                !params.windowStep.has_value())
            {
                throw std::invalid_argument("density requires layer, value, limit, window size, and window step");
            }

            return std::make_unique<DensityRule>(params.layer.value(), params.densityLimit.value(), params.value.value(), params.windowSize.value(), params.windowStep.value(), params.analysisWindow);
        }

        throw std::invalid_argument("Unknown rule type: " + type);
    }

}
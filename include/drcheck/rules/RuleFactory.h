#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "drcheck/domain/Layer.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/rules/Rule.h"
#include "drcheck/rules/DensityRule.h"
#include "drcheck/rules/MinEnclosureRule.h"

namespace drcheck::rules {

    struct RuleParameters
    {
        const domain::Layer* layer = nullptr;

        const domain::Layer* innerLayer = nullptr;
        const domain::Layer* outerLayer = nullptr;

        std::optional<double> value;
        
        // Density rule params
        std::optional<DensityLimit> densityLimit;
        std::optional<double> windowSize;
        std::optional<double> windowStep;
        std::optional<geometry::BoundingBox> analysisWindow;

        // MinEnclosure rule params
        std::optional<std::vector<EnclosureOption>> enclosureOptions;
    };

    class RuleFactory
    {
    public:
        static std::unique_ptr<Rule> create(const std::string& type, const RuleParameters& params);
    };

}
#pragma once

#include <memory>
#include <optional>
#include <string>

#include "drcheck/domain/Layer.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/rules/Rule.h"
#include "drcheck/rules/DensityRule.h"

namespace drcheck::rules {

    struct RuleParameters
    {
        std::optional<domain::Layer> layer;

        std::optional<domain::Layer> innerLayer;
        std::optional<domain::Layer> outerLayer;

        std::optional<double> value;

        std::optional<DensityLimit> densityLimit;
        std::optional<double> windowSize;
        std::optional<double> windowStep;

        std::optional<geometry::BoundingBox> analysisWindow;
    };

    class RuleFactory
    {
    public:
        static std::unique_ptr<Rule> create(const std::string& type, const RuleParameters& params);
    };

}
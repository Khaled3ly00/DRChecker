#pragma once

#include <vector>
#include <optional>
#include <cstddef>

#include "Rule.h"
#include "drcheck/domain/Layer.h"

namespace drcheck::rules {
class EnclosureOption
{
public:
    EnclosureOption(domain::Layer outerLayer, double allSidesMinEnclosure);
    EnclosureOption(domain::Layer outerLayer, double allSidesMinEnclosure, double firstPairMinEnclosure, double secondPairMinEnclosure);

    domain::Layer getOuterLayer() const;
    bool hasOppositeEnclosureRequirement() const;
    double getFirstPairMinEnclosure() const;
    double getSecondPairMinEnclosure() const;
    double getAllSidesMinEnclosure() const;

private:
    domain::Layer outerLayer;
    double allSidesMinEnclosure;
    std::optional<double> firstPairMinEnclosure;
    std::optional<double> secondPairMinEnclosure;
};

class MinEnclosureRule : public Rule
{
public:
    MinEnclosureRule(domain::Layer innerLayer, domain::Layer outerLayer, double allSidesMinEnclosure);
    MinEnclosureRule(domain::Layer innerLayer, std::vector<EnclosureOption> enclosureOptions);

    std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const override;

    domain::Layer getInnerLayer() const;
    domain::Layer getOuterLayer() const; // legacy compatibility
    double getMinimumEnclosure() const; // legacy compatibility

    std::size_t getEnclosureOptionCount() const;
    const EnclosureOption& getEnclosureOption(std::size_t optionNumber) const;

private:
    domain::Layer innerLayer;
    std::vector<EnclosureOption> enclosureOptions;
};
}
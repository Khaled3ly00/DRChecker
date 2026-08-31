#include "drcheck/rules/MinEnclosureRule.h"
#include "drcheck/rules/Constants.h"
#include "drcheck/domain/Layer.h"
#include "drcheck/spatial/LayerSpatialIndex.h"

#include <stdexcept>
#include <optional>
#include <limits>

namespace drcheck::rules {

EnclosureOption::EnclosureOption(domain::Layer outerLayer, double allSidesMinEnclosure)
    : outerLayer(outerLayer), allSidesMinEnclosure(allSidesMinEnclosure), firstPairMinEnclosure(std::nullopt), secondPairMinEnclosure(std::nullopt)
{
    // If Minenclosure is 0 then intersecting inner shapes won't create a violation
    // To support that disable checking for intersecting shapes
    if (allSidesMinEnclosure <= 0.0) {
        throw std::invalid_argument("Minimum enclosure cannot be negative");
    }
}
// Two opposite sides option constructor
EnclosureOption::EnclosureOption(domain::Layer outerLayer, double allSidesMinEnclosure, double firstPairMinEnclosure, double secondPairMinEnclosure)
    : outerLayer(outerLayer), allSidesMinEnclosure(allSidesMinEnclosure), firstPairMinEnclosure(firstPairMinEnclosure), secondPairMinEnclosure(secondPairMinEnclosure)
{
    if (firstPairMinEnclosure < 0.0 || secondPairMinEnclosure < 0.0 || allSidesMinEnclosure < 0.0)
    {
        throw std::invalid_argument("Enclosure values cannot be negative");
    }

}
bool EnclosureOption::hasOppositeEnclosureRequirement() const
{
   return firstPairMinEnclosure.has_value();
}

double EnclosureOption::getFirstPairMinEnclosure() const
{
    if (!firstPairMinEnclosure.has_value())
    {
        throw std::logic_error("Enclosure option has no opposite-side requirement");
    }

    return firstPairMinEnclosure.value();
}

double EnclosureOption::getSecondPairMinEnclosure() const
{
    if (!secondPairMinEnclosure.has_value())
    {
        throw std::logic_error("Enclosure option has no opposite-side requirement");
    }

    return secondPairMinEnclosure.value();
}

double EnclosureOption::getAllSidesMinEnclosure() const
{
    return allSidesMinEnclosure;
}

domain::Layer EnclosureOption::getOuterLayer() const
{
    return outerLayer;
}

// Delegate constructor for backward compatability 
MinEnclosureRule::MinEnclosureRule(domain::Layer innerLayer, domain::Layer outerLayer, double allSidesMinEnclosure)
    : MinEnclosureRule(innerLayer, std::vector<EnclosureOption>{EnclosureOption(outerLayer, allSidesMinEnclosure)})
{
}

MinEnclosureRule::MinEnclosureRule(domain::Layer innerLayer, std::vector<EnclosureOption> enclosureOptions) 
    : innerLayer(innerLayer), enclosureOptions(std::move(enclosureOptions))
{
    if (this->enclosureOptions.empty())
    {
        throw std::invalid_argument("MinEnclosureRule requires at least one enclosure option");
    }

    for (const EnclosureOption& option : this->enclosureOptions)
    {
        if (innerLayer == option.getOuterLayer())
        {
            throw std::invalid_argument("Enclosure rule requires different inner and outer layers");
        }
    }
}

std::vector<domain::Violation> MinEnclosureRule::check(const std::vector<domain::Shape>& shapes, const spatial::LayerSpatialIndex& spatialIndex) const
{
    std::vector<domain::Violation> violations;

    for (const domain::Shape& innerShape : shapes)
    {
        if (innerShape.getLayer() != innerLayer) {
            continue;
        }

        bool validEnclosureFound = false;

        const domain::Shape* bestFailedOuterShape = nullptr;
        std::optional<geometry::PolygonEdgePairResult> bestFailedEnclosure;

        double bestRequiredEnclosure = 0.0;
        double bestActualEnclosure = 0.0;
        double smallestDeficit = std::numeric_limits<double>::infinity();

        const auto innerBox = innerShape.getPolygon().getBoundingBox();

        // loop through all options (layer, minenclosure) pairs
        for (const EnclosureOption& option : enclosureOptions)
        {
            // query for shapes within bBox with same layer
            const auto candidates = spatialIndex.query(option.getOuterLayer(), innerBox);

            for (const domain::Shape* outerShape : candidates)
            {
                const auto& innerPolygon = innerShape.getPolygon();
                const auto& outerPolygon = outerShape->getPolygon();

                const bool containsInner = outerPolygon.contains(innerPolygon, true);
                const bool overlapsInner = outerPolygon.overlaps(innerPolygon);
                const bool intersectsInner = outerPolygon.intersects(innerPolygon);

                // Outer non related polygon skip
                if (!containsInner && !overlapsInner && !intersectsInner)
                {
                    continue;
                }
                // Inner polygon crosses outside the outer polygon.
                // This is an enclosure failure with actual enclosure = 0.
                if (!containsInner && !option.hasOppositeEnclosureRequirement())
                {
                    const double requiredEnclosure = option.getAllSidesMinEnclosure();

                    if (requiredEnclosure < smallestDeficit)
                    {
                        smallestDeficit = requiredEnclosure;

                        bestFailedOuterShape = outerShape;
                        bestActualEnclosure = 0.0;
                        bestRequiredEnclosure = requiredEnclosure;

                        bestFailedEnclosure = innerPolygon.distanceTo(outerPolygon, false);
                    }

                    continue;
                }
                // Inner contained polygon with AllSidesMinEnclosure
                if (!option.hasOppositeEnclosureRequirement())
                {
                    const geometry::PolygonEdgePairResult actualEnclosure = innerPolygon.distanceTo(outerPolygon, false);
                    const double requiredEnclosure = option.getAllSidesMinEnclosure();

                    if (actualEnclosure.distance + DRC_LENGTH_TOLERANCE >= requiredEnclosure)
                    {
                        validEnclosureFound = true;
                        break;
                    }

                    const double deficit = requiredEnclosure - actualEnclosure.distance;

                    if (deficit < smallestDeficit)
                    {
                        smallestDeficit = deficit;

                        bestFailedOuterShape = outerShape;
                        bestFailedEnclosure = actualEnclosure;

                        bestActualEnclosure = actualEnclosure.distance;
                        bestRequiredEnclosure = requiredEnclosure;
                    }
                    continue;
                }
                // Inner contained polygon with Opposite Sides Enclosure
                if (option.hasOppositeEnclosureRequirement()) {
                    geometry::PairwiseEnclosureResult enclosure = innerPolygon.pairwiseEnclosure(outerPolygon);

                    const double allSidesRequired = option.getAllSidesMinEnclosure();
                    const double firstPairRequired = option.getFirstPairMinEnclosure();
                    const double secondPairRequired = option.getSecondPairMinEnclosure();

                    if (enclosure.left < 0.0 && enclosure.left >= -DRC_LENGTH_TOLERANCE)
                    {
                        enclosure.left = 0.0;
                    }
                    if (enclosure.right < 0.0 && enclosure.right >= -DRC_LENGTH_TOLERANCE)
                    {
                        enclosure.right = 0.0;
                    }
                    if (enclosure.bottom < 0.0 && enclosure.bottom >= -DRC_LENGTH_TOLERANCE)
                    {
                        enclosure.bottom = 0.0;
                    }
                    if (enclosure.top < 0.0 && enclosure.top >= -DRC_LENGTH_TOLERANCE)
                    {
                        enclosure.top = 0.0;
                    }

                    if (enclosure.left < 0.0 || enclosure.right < 0.0 || enclosure.bottom < 0.0 || enclosure.top < 0.0)
                    {
                        const double requiredEnclosure = std::max({allSidesRequired, firstPairRequired, secondPairRequired});
                        const double deficit = requiredEnclosure;

                        if (deficit < smallestDeficit)
                        {
                            smallestDeficit = deficit;

                            bestFailedOuterShape = outerShape;
                            bestActualEnclosure = 0.0;
                            bestRequiredEnclosure = requiredEnclosure;

                            bestFailedEnclosure = innerPolygon.distanceTo(outerPolygon, false);
                        }

                        continue;
                    }

                    // Check all sides min enclosure option first
                    const double minimumSide = std::min({ enclosure.left, enclosure.right, enclosure.bottom, enclosure.top });
                    if (minimumSide + DRC_LENGTH_TOLERANCE >= allSidesRequired)
                    {
                        validEnclosureFound = true;
                        break;
                    }
                    // Check opposite sides as all sides min enclosure failed
                    const double leftRight = std::min(enclosure.left, enclosure.right);
                    const double topBottom = std::min(enclosure.top, enclosure.bottom);

                    const bool firstOrientationPasses =
                        leftRight + DRC_LENGTH_TOLERANCE >= firstPairRequired &&
                        topBottom + DRC_LENGTH_TOLERANCE >= secondPairRequired;
                    const bool secondOrientationPasses =
                        topBottom + DRC_LENGTH_TOLERANCE >= firstPairRequired &&
                        leftRight + DRC_LENGTH_TOLERANCE >= secondPairRequired;

                    if (firstOrientationPasses || secondOrientationPasses)
                    {
                        validEnclosureFound = true;
                        break;
                    }

                    double optionActualValue = minimumSide;
                    double optionRequiredValue = allSidesRequired;
                    double optionDeficit = optionRequiredValue - optionActualValue;

                    // Calculate first orientation deficit:
                    // left/right -> first pair, top/bottom -> second pair
                    const double firstPairDeficit = std::max(0.0, firstPairRequired - leftRight);
                    const double secondPairDeficit = std::max(0.0, secondPairRequired - topBottom);

                    double firstOrientationActual;
                    double firstOrientationRequired;

                    if (firstPairDeficit >= secondPairDeficit)
                    {
                        firstOrientationActual = leftRight;
                        firstOrientationRequired = firstPairRequired;
                    }
                    else
                    {
                        firstOrientationActual = topBottom;
                        firstOrientationRequired = secondPairRequired;
                    }

                    const double firstOrientationDeficit = std::max(firstPairDeficit, secondPairDeficit);

                    if (firstOrientationDeficit < optionDeficit)
                    {
                        optionDeficit = firstOrientationDeficit;
                        optionActualValue = firstOrientationActual;
                        optionRequiredValue = firstOrientationRequired;
                    }

                    // Calculate second orientation deficit:
                    // top/bottom -> first pair, left/right -> second pair
                    const double rotatedFirstPairDeficit = std::max(0.0, firstPairRequired - topBottom);
                    const double rotatedSecondPairDeficit = std::max(0.0, secondPairRequired - leftRight);

                    double secondOrientationActual;
                    double secondOrientationRequired;

                    if (rotatedFirstPairDeficit >= rotatedSecondPairDeficit)
                    {
                        secondOrientationActual = topBottom;
                        secondOrientationRequired = firstPairRequired;
                    }
                    else
                    {
                        secondOrientationActual = leftRight;
                        secondOrientationRequired = secondPairRequired;
                    }

                    const double secondOrientationDeficit = std::max(rotatedFirstPairDeficit, rotatedSecondPairDeficit);

                    if (secondOrientationDeficit < optionDeficit)
                    {
                        optionDeficit = secondOrientationDeficit;
                        optionActualValue = secondOrientationActual;
                        optionRequiredValue = secondOrientationRequired;
                    }

                    // Compare best failure from this option against failures
                    // from other outer shapes and enclosure options
                    if (optionDeficit < smallestDeficit)
                    {
                        smallestDeficit = optionDeficit;

                        bestFailedOuterShape = outerShape;

                        bestActualEnclosure = optionActualValue;
                        bestRequiredEnclosure = optionRequiredValue;

                        bestFailedEnclosure = innerPolygon.distanceTo(outerPolygon, false);
                    }
                }
            }

            if (validEnclosureFound)
            {
                break;
            }
        }

        if (validEnclosureFound)
        {
            continue;
        }

        if (bestFailedOuterShape != nullptr && bestFailedEnclosure.has_value())
        {
            const auto& actualEnclosure = bestFailedEnclosure.value();

            domain::ViolationMarker marker{
                .firstPoint = actualEnclosure.firstPoint,
                .secondPoint = actualEnclosure.secondPoint,
                .firstEdgeIndex = actualEnclosure.firstEdgeIndex,
                .secondEdgeIndex = actualEnclosure.secondEdgeIndex,
                .firstLayer = getInnerLayer(),
                .secondLayer = bestFailedOuterShape->getLayer()
            };

            const std::string msg = "Minimum Enclosure violation: " + domain::layerToString(getInnerLayer()) + " enclosed by " + domain::layerToString(bestFailedOuterShape->getLayer())
                + " should be " + std::to_string(bestRequiredEnclosure) + " actual " + std::to_string(bestActualEnclosure);

            violations.emplace_back(domain::ViolationType::Enclosure, std::vector<std::size_t>{ innerShape.getId(), bestFailedOuterShape->getId()},
                msg, bestActualEnclosure, bestRequiredEnclosure, marker);
            continue;
        }

        domain::ViolationMarker marker{ .firstLayer = getInnerLayer() };
        const std::string msg = domain::layerToString(getInnerLayer()) + " is not enclosed by any allowed outer layer";

        violations.emplace_back(domain::ViolationType::Enclosure, std::vector<std::size_t>{ innerShape.getId() },
          msg, 0.0, 0.0, marker);
    }
    return violations;
}

domain::Layer MinEnclosureRule::getInnerLayer() const
{
    return innerLayer;
}

domain::Layer MinEnclosureRule::getOuterLayer() const
{
    return enclosureOptions.front().getOuterLayer();
}

double MinEnclosureRule::getMinimumEnclosure() const
{
    return enclosureOptions.front().getAllSidesMinEnclosure();
}

std::size_t MinEnclosureRule::getEnclosureOptionCount() const
{
    return enclosureOptions.size();
}

const EnclosureOption& MinEnclosureRule::getEnclosureOption(std::size_t optionNumber) const
{
    return enclosureOptions.at(optionNumber);
}

}
#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <optional>

#include "drcheck/geometry/Point.h"

namespace drcheck::domain {
struct ViolationMarker
{
    // For two-shape violations:
    // first fields correspond to shapeIds[0],
    // second fields correspond to shapeIds[1].
    //
    // For one-shape violations: (MinWidth)
    // both fields correspond to shapeIds[0].
    geometry::Point firstPoint;
    geometry::Point secondPoint;

    std::optional<std::size_t> firstEdgeIndex;
    std::optional<std::size_t> secondEdgeIndex;
};
enum class ViolationType
{
    MinWidth,
    MinSpacing,
    Enclosure
};

class Violation
{
public:
    Violation(ViolationType type, std::vector<std::size_t> shapeIds, std::string message, double actualValue, double requiredValue, std::optional<ViolationMarker> marker = std::nullopt);

    ViolationType getType() const;
    std::string getTypeAsString() const;

    const std::vector<std::size_t>& getShapeIds() const;

    const std::string& getMessage() const;
    double getActualValue() const;
    double getRequiredValue() const;
    const std::optional<ViolationMarker>& getMarker() const;
private:
    ViolationType type;
	// shapeIds is a vector of shape IDs that are involved in the violation
	// Can be one or more shapes depending on the violation type
    std::vector<std::size_t> shapeIds;
    std::string message;
    double actualValue;
    double requiredValue;
    std::optional<ViolationMarker> marker;
};

}
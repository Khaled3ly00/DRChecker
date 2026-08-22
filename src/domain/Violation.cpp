#include "drcheck/domain/Violation.h"

#include <utility>
#include <stdexcept>

namespace drcheck::domain {

Violation::Violation(ViolationType type, std::vector<std::size_t> shapeIds, std::string message, double actualValue, double requiredValue, std::optional<ViolationMarker> marker)
        : type(type), shapeIds(std::move(shapeIds)), message(std::move(message)), actualValue(actualValue), requiredValue(requiredValue), marker(std::move(marker))
{
}

ViolationType Violation::getType() const
{
    return type;
}

std::string Violation::getTypeAsString() const {
    switch (type)
    {
    case ViolationType::MinWidth:
        return "MinWidth";

    case ViolationType::MinSpacing:
        return "MinSpacing";

    case ViolationType::Enclosure:
        return "Enclosure";

    case ViolationType::MinDensity:
        return "MinDensity";

    case ViolationType::MaxDensity:
        return "MaxDensity";
    }
    throw std::logic_error("Unknown violation type");
}

const std::vector<std::size_t>& Violation::getShapeIds() const
{
    return shapeIds;
}

const std::string& Violation::getMessage() const
{
    return message;
}
double Violation::getActualValue() const
{
    return actualValue;
}

double Violation::getRequiredValue() const
{
    return requiredValue;
}
const std::optional<ViolationMarker>& Violation::getMarker() const {
    return marker;
}
}
#include "drcheck/domain/Violation.h"

#include <utility>

namespace drcheck::domain {

Violation::Violation(ViolationType type,std::vector<std::size_t> shapeIds,std::string message)
        : type(type), shapeIds(std::move(shapeIds)), message(std::move(message))
{
}

ViolationType Violation::getType() const
{
    return type;
}

const std::vector<std::size_t>& Violation::getShapeIds() const
{
    return shapeIds;
}

const std::string& Violation::getMessage() const
{
    return message;
}

}
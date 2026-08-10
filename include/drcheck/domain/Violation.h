#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace drcheck::domain {
enum class ViolationType
{
    MinWidth,
    MinSpacing,
    Enclosure
};

class Violation
{
public:
    Violation(ViolationType type, std::vector<std::size_t> shapeIds, std::string message);

    ViolationType getType() const;

    const std::vector<std::size_t>& getShapeIds() const;

    const std::string& getMessage() const;

private:
    ViolationType type;
    std::vector<std::size_t> shapeIds;
    std::string message;
};

}
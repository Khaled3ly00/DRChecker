#pragma once

#include <vector>

#include "drcheck/domain/Shape.h"
#include "drcheck/domain/Violation.h"

namespace drcheck::rules {

class Rule
{
public:
    virtual ~Rule() = default;

    virtual std::vector<domain::Violation> check(const std::vector<domain::Shape>& shapes) const = 0;
};

}
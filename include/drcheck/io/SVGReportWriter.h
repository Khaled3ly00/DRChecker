#pragma once

#include "drcheck/domain/Violation.h"
#include "drcheck/domain/Shape.h"

#include <string>
#include <vector>

namespace drcheck::io {

class SVGReportWriter
{
public:
    static void write(const std::vector<domain::Shape>& shapes, const std::vector<domain::Violation>& violations, const std::string& filePath);
};
}
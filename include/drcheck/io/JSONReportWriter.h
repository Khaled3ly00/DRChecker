#pragma once

#include <string>
#include <vector>

#include "drcheck/domain/Violation.h"

namespace drcheck::io {

    class JSONReportWriter
    {
    public:
        void write(const std::vector<domain::Violation>& violations, const std::string& filePath) const;
    };

}
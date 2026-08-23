#pragma once

#include <string>
#include <vector>

#include "drcheck/domain/Violation.h"

namespace drcheck::io {

    class TclAutomationRunner
    {
    public:
        static std::vector<domain::Violation> run(const std::string& filePath);
    };

}
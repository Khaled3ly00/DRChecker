#pragma once

#include <string>

#include "drcheck/domain/Shape.h"

namespace drcheck::io {

class JSONLayoutParser
{
public:
    std::vector<domain::Shape> load(const std::string& filePath) const;
};

}
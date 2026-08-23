#pragma once

#include <string>

#include "drcheck/domain/Shape.h"

namespace drcheck::io {

class JSONLayoutParser
{
public:
    static std::vector<domain::Shape> load(const std::string& filePath);
};

}
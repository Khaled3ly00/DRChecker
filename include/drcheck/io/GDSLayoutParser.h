#pragma once

#include <string>
#include <vector>
#include <optional>

#include "drcheck/domain/LayerRegistry.h"
#include "drcheck/domain/Shape.h"

namespace drcheck::io {

    class GDSLayoutParser
    {
    public:
        static std::vector<domain::Shape> load(const std::string& filePath, const domain::LayerRegistry& layerRegistry, const std::optional<std::string>& topCellName = std::nullopt);
    };

}
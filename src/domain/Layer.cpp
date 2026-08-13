#include "drcheck/domain/Layer.h"

#include <stdexcept>

namespace drcheck::domain {
Layer layerFromString(const std::string& layerName)
{
    if (layerName == "Metal1") {
        return Layer::Metal1;
    }

    if (layerName == "Metal2") {
        return Layer::Metal2;
    }

    if (layerName == "Poly") {
        return Layer::Poly;
    }

    if (layerName == "Diffusion") {
        return Layer::Diffusion;
    }

    if (layerName == "Via12") {
        return Layer::Via12;
    }

    throw std::invalid_argument(
        "Unknown layer: " + layerName
    );
}
}
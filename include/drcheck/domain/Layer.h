#pragma once

#include <string>

namespace drcheck::domain {

    enum class Layer
    {
        Metal1,
        Metal2,
        Poly,
        Diffusion,
        Via12
    };

    Layer layerFromString(const std::string& layerName);
}
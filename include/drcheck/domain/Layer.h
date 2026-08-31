#pragma once

#include <string>

namespace drcheck::domain {

    enum class Layer
    {
        NW,
        NP,
        PP,
        M1,
        M1_PIN,
        M2,
        M2_PIN,
        PO,
        OD,
        CO,
        VIA1,
        PDK,
        VTL_N,
        VTL_P
    };

    enum class Purpose
    {
        Drawing,
        Pin
    };

    Layer layerFromString(const std::string& layerName);

    Purpose purposeFromString(const std::string& purposeName);

    std::string layerToString(Layer layer);
}
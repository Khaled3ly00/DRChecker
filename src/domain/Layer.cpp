#include "drcheck/domain/Layer.h"

#include <stdexcept>

namespace drcheck::domain {
Layer layerFromString(const std::string& layerName)
{
    if (layerName == "NW") {
        return Layer::NW;
    }

    if (layerName == "NP") {
        return Layer::NP;
    }

    if (layerName == "PP") {
        return Layer::PP;
    }

    if (layerName == "M1") {
        return Layer::M1;
    }

    if (layerName == "M1_PIN") {
        return Layer::M1_PIN;
    }

    if (layerName == "M2") {
        return Layer::M2;
    }

    if (layerName == "M2_PIN") {
        return Layer::M2_PIN;
    }

    if (layerName == "PO") {
        return Layer::PO;
    }

    if (layerName == "OD") {
        return Layer::OD;
    }

    if (layerName == "CO") {
        return Layer::CO;
    }

    if (layerName == "VIA1") {
        return Layer::VIA1;
    }

    if (layerName == "PDK") {
        return Layer::PDK;
    }

    if (layerName == "VTL_N") {
        return Layer::VTL_N;
    }

    if (layerName == "VTL_P") {
        return Layer::VTL_P;
    }

    throw std::invalid_argument(
        "Unknown layer: " + layerName
    );
}

Purpose purposeFromString(const std::string& purposeName) {
    if (purposeName == "drawing") {
        return Purpose::Drawing;
    }

    if (purposeName == "pin") {
        return Purpose::Pin;
    }

    throw std::invalid_argument(
        "Unknown purpose: " + purposeName
    );
}
}
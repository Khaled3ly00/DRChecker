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

std::string layerToString(Layer layer)
{
    switch (layer)
    {
    case Layer::NW:
        return "NW";

    case Layer::NP:
        return "NP";

    case Layer::PP:
        return "PP";

    case Layer::M1:
        return "M1";

    case Layer::M1_PIN:
        return "M1_PIN";

    case Layer::M2:
        return "M2";

    case Layer::M2_PIN:
        return "M2_PIN";

    case Layer::PO:
        return "PO";

    case Layer::OD:
        return "OD";

    case Layer::CO:
        return "CO";

    case Layer::VIA1:
        return "VIA1";
    
    case Layer::PDK:
        return "PDK";

    case Layer::VTL_N:
        return "VTL_N";

    case Layer::VTL_P:
        return "VTL_P";
    }

    throw std::invalid_argument("Unknown layer");
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
#include "drcheck/io/JSONReportWriter.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace {
    double roundForReport(double value)
    {
        constexpr double scale = 1'000'000.0;
        return std::round(value * scale) / scale;
    }
}

namespace drcheck::io {
void JSONReportWriter::write(const std::vector<domain::Violation>& violations, const std::string& filePath)
{
    nlohmann::ordered_json report;
    // Create JSON element violationCount
    report["violationCount"] = violations.size();

    // Create JSON array element violations
    report["violations"] = nlohmann::ordered_json::array();

    for (const auto& violation : violations)
    {
        // Create JSON array element violationJson
        nlohmann::ordered_json violationJson;

        violationJson["type"] = violation.getTypeAsString();
        violationJson["shapeIds"] = violation.getShapeIds();
        violationJson["message"] = violation.getMessage();
        violationJson["actual"] = roundForReport(violation.getActualValue());
        violationJson["required"] = violation.getRequiredValue();

        if (violation.getMarker().has_value())
        {
            const auto& marker = violation.getMarker().value();
            if (marker.firstPoint.has_value())
            {
                violationJson["marker"]["firstPoint"] = {
                    {"x", roundForReport(marker.firstPoint->getX())},
                    {"y", roundForReport(marker.firstPoint->getY())}
                };
            }

            if (marker.secondPoint.has_value())
            {
                violationJson["marker"]["secondPoint"] = {
                    {"x", roundForReport(marker.secondPoint->getX())},
                    {"y", roundForReport(marker.secondPoint->getY())}
                };
            }
            if (marker.firstEdgeIndex.has_value())
            {
                violationJson["marker"]["firstEdgeIndex"] = marker.firstEdgeIndex.value();
            }

            if (marker.secondEdgeIndex.has_value())
            {
                violationJson["marker"]["secondEdgeIndex"] = marker.secondEdgeIndex.value();
            }
            if (marker.region.has_value())
            {
                violationJson["marker"]["region"] = {
                    {
                        "minX",
                        roundForReport(marker.region->getMinX())
                    },
                    {
                        "minY",
                        roundForReport(marker.region->getMinY())
                    },
                    {
                        "maxX",
                        roundForReport(marker.region->getMaxX())
                    },
                    {
                        "maxY",
                        roundForReport(marker.region->getMaxY())
                    }
                };
            }
            if (marker.firstLayer.has_value()) {
                violationJson["marker"]["firstLayer"] = {"firstLayer", marker.firstLayer.value()};
            }
            if (marker.secondLayer.has_value()) {
                violationJson["marker"]["secondLayer"] = { "firstLayer", marker.secondLayer.value() };
            }
        }
        // Push violationJson to violations
        report["violations"].push_back(std::move(violationJson));
    }

    std::ofstream output(filePath);

    if (!output) {
        throw std::runtime_error("Unable to open report file: " + filePath);
    }
    // pretty print
    output << report.dump(4);
}
}
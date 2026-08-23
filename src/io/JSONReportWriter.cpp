#include "drcheck/io/JSONReportWriter.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

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
        violationJson["actual"] = violation.getActualValue();
        violationJson["required"] = violation.getRequiredValue();

        if (violation.getMarker().has_value())
        {
            const auto& marker = violation.getMarker().value();
            if (marker.firstPoint.has_value())
            {
                violationJson["marker"]["firstPoint"] = {
                    {"x", marker.firstPoint->getX()},
                    {"y", marker.firstPoint->getY()}
                };
            }

            if (marker.secondPoint.has_value())
            {
                violationJson["marker"]["secondPoint"] = {
                    {"x", marker.secondPoint->getX()},
                    {"y", marker.secondPoint->getY()}
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
                        marker.region->getMinX()
                    },
                    {
                        "minY",
                        marker.region->getMinY()
                    },
                    {
                        "maxX",
                        marker.region->getMaxX()
                    },
                    {
                        "maxY",
                        marker.region->getMaxY()
                    }
                };
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
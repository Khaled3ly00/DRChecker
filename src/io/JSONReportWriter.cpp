#include "drcheck/io/JSONReportWriter.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace drcheck::io {
void JSONReportWriter::write(const std::vector<domain::Violation>& violations, const std::string& filePath) const
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
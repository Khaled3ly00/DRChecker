#include "drcheck/io/JSONLayoutParser.h"

#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace drcheck::io {
std::vector<domain::Shape> JSONLayoutParser::load(const std::string& filePath) {
    std::ifstream input(filePath);

    if (!input) {
        throw std::runtime_error("Unable to open layout file: " + filePath);
    }

    nlohmann::json json;
    input >> json;

    // Array shapes must exist in JSON File
    if (!json.contains("shapes") || !json["shapes"].is_array())
    {
        throw std::invalid_argument("Layout must contain a shapes array");
    }

    std::vector<domain::Shape> shapes;
    shapes.reserve(json["shapes"].size());

    // Create unique ID for each imported shape
    std::size_t id = 0;

    // For loop to extract each shape from JSON File
    for (const auto& shapeJson : json["shapes"])
    {
        if (!shapeJson.is_object()) {
            throw std::invalid_argument("Each shape must be a JSON object");
        }
        // Extracting Layer as string then calling parseLayer
        const domain::Layer layer = domain::layerFromString(shapeJson.at("layer").get<std::string>());

        // Extracting Vertices
        std::vector<geometry::Point> vertices;
        const auto& verticesJson = shapeJson.at("vertices");
        vertices.reserve(verticesJson.size());

        // Is the extracted data an array?
        if (!verticesJson.is_array()) {
            throw std::invalid_argument("vertices must be an array");
        }
        // Loop through vertices array
        for (const auto& vertexJson : verticesJson)
        {
            // Each element of vertices must be an array with size = 2 (x, y)
            if (!vertexJson.is_array() || vertexJson.size() != 2)
            {
                throw std::invalid_argument(
                    "Vertex must be [x, y]"
                );
            }
            vertices.emplace_back(vertexJson[0].get<double>(), vertexJson[1].get<double>());
        }
        
        // Create Polygon using Extracted Vertices
        geometry::Polygon polygon(std::move(vertices));

        // Create Shape
        shapes.emplace_back(id, layer, std::move(polygon));
        ++id;
    }
    return shapes;
}
}
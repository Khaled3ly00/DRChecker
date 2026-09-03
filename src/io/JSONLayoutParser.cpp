#include "drcheck/io/JSONLayoutParser.h"

#include <fstream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace drcheck::io {

    std::vector<domain::Shape> JSONLayoutParser::load(const std::string& filePath, const domain::LayerRegistry& layerRegistry)
    {
        std::ifstream input(filePath);

        if (!input)
        {
            throw std::runtime_error("Unable to open layout file: " + filePath);
        }

        nlohmann::json json;
        input >> json;

        if (!json.contains("units"))
        {
            throw std::invalid_argument("Layout must contain a units field");
        }

        if (!json["units"].is_string())
        {
            throw std::invalid_argument("Layout units must be a string");
        }

        if (json["units"].get<std::string>() != "um")
        {
            throw std::invalid_argument("Unsupported layout units. Expected \"um\"");
        }

        if (!json.contains("shapes") || !json["shapes"].is_array())
        {
            throw std::invalid_argument("Layout must contain a shapes array");
        }

        std::vector<domain::Shape> shapes;
        shapes.reserve(json["shapes"].size());

        for (const auto& shapeJson : json["shapes"])
        {
            if (!shapeJson.is_object())
            {
                throw std::invalid_argument("Each shape must be a JSON object");
            }

            if (!shapeJson.contains("id"))
            {
                throw std::invalid_argument("Shape is missing required field: id");
            }

            if (!shapeJson.contains("layer"))
            {
                throw std::invalid_argument("Shape is missing required field: layer");
            }

            if (!shapeJson.contains("vertices"))
            {
                throw std::invalid_argument("Shape is missing required field: vertices");
            }

            if (!shapeJson["id"].is_number_unsigned() && !shapeJson["id"].is_number_integer())
            {
                throw std::invalid_argument("Shape id must be an integer");
            }

            const std::size_t id = shapeJson["id"].get<std::size_t>();

            if (!shapeJson["layer"].is_string())
            {
                throw std::invalid_argument("Shape layer must be a string");
            }

            const domain::Layer* layer = layerRegistry.resolve(shapeJson["layer"].get<std::string>());

            const auto& verticesJson = shapeJson["vertices"];

            if (!verticesJson.is_array())
            {
                throw std::invalid_argument("vertices must be an array");
            }

            std::vector<geometry::Point> vertices;
            vertices.reserve(verticesJson.size());

            for (const auto& vertexJson : verticesJson)
            {
                if (!vertexJson.is_array() || vertexJson.size() != 2)
                {
                    throw std::invalid_argument("Vertex must be [x, y]");
                }

                if (!vertexJson[0].is_number() || !vertexJson[1].is_number())
                {
                    throw std::invalid_argument("Vertex coordinates must be numbers");
                }

                vertices.emplace_back(vertexJson[0].get<double>(), vertexJson[1].get<double>());
            }

            geometry::Polygon polygon(std::move(vertices));

            shapes.emplace_back(id, layer, std::move(polygon));
        }

        return shapes;
    }

}

#include "drcheck/io/SVGReportWriter.h"

#include <fstream>
#include <stdexcept>
#include <optional>

namespace drcheck::io {
    namespace {

        constexpr double SVG_SCALE = 20.0;
        constexpr double SVG_PADDING = 20.0;

        constexpr double SHAPE_STROKE_WIDTH = 1.0;
        constexpr double VIOLATION_EDGE_WIDTH = 2.0;
        constexpr double VIOLATION_MARKER_WIDTH = 2.0;
        constexpr double VIOLATION_POINT_RADIUS = 2.5;

        constexpr double SHAPE_ID_FONT_SIZE = 4.0;
        constexpr double VIOLATION_LABEL_FONT_SIZE = 6.0;

        // Return bounding box that contains both layout shapes and violation regions.
        std::optional<geometry::BoundingBox> calculateReportBounds(const std::vector<domain::Shape>& shapes, const std::vector<domain::Violation>& violations)
        {
            std::optional<geometry::BoundingBox> bounds;

            for (const auto& shape : shapes)
            {
                const geometry::BoundingBox shapeBounds = shape.getPolygon().getBoundingBox();

                if (!bounds.has_value())
                {
                    bounds = shapeBounds;
                }
                else
                {
                    bounds = bounds->mergedWith(shapeBounds);
                }
            }

            for (const auto& violation : violations)
            {
                if (!violation.getMarker().has_value())
                {
                    continue;
                }

                const auto& marker = violation.getMarker().value();

                if (!marker.region.has_value())
                {
                    continue;
                }

                if (!bounds.has_value())
                {
                    bounds = marker.region.value();
                }
                else
                {
                    bounds = bounds->mergedWith(marker.region.value());
                }
            }

            return bounds;
        }

        // Converts Layout X coordinate to SVG X coordinate.
        double toSvgX(double layoutX, const geometry::BoundingBox& bounds)
        {
            return (layoutX - bounds.getMinX()) * SVG_SCALE + SVG_PADDING;
        }

        // Converts Layout Y coordinate to SVG Y coordinate.
        double toSvgY(double layoutY, const geometry::BoundingBox& bounds)
        {
            return (bounds.getMaxY() - layoutY) * SVG_SCALE + SVG_PADDING;
        }

        std::string layerFillColor(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::Metal1:
                return "turquoise";

            case domain::Layer::Metal2:
                return "gold";

            case domain::Layer::Via12:
                return "yellow";

            case domain::Layer::Poly:
                return "blue";

            case domain::Layer::Diffusion:
                return "crimson";
            }

            return "lightgray";
        }

        std::string layerStrokeColor(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::Metal1:
                return "darkturquoise";

            case domain::Layer::Metal2:
                return "goldenrod";

            case domain::Layer::Via12:
                return "khaki";

            case domain::Layer::Poly:
                return "navy";

            case domain::Layer::Diffusion:
                return "red";
            }

            return "black";
        }

        void writeHighlightedEdge(std::ofstream& output, const geometry::Segment& edge, const geometry::BoundingBox& bounds)
        {
            const double x1 = toSvgX(edge.getStart().getX(), bounds);
            const double y1 = toSvgY(edge.getStart().getY(), bounds);
            const double x2 = toSvgX(edge.getEnd().getX(), bounds);
            const double y2 = toSvgY(edge.getEnd().getY(), bounds);

            output
                << "    <line "
                << "class=\"violation-edge\" "
                << "x1=\"" << x1 << "\" "
                << "y1=\"" << y1 << "\" "
                << "x2=\"" << x2 << "\" "
                << "y2=\"" << y2 << "\" "
                << "stroke=\"red\" "
                << "stroke-width=\""
                << VIOLATION_EDGE_WIDTH
                << "\" "
                << "stroke-linecap=\"round\""
                << " />\n";
        }

        const domain::Shape* findShapeById(const std::vector<domain::Shape>& shapes, std::size_t id)
        {
            for (const auto& shape : shapes)
            {
                if (shape.getId() == id)
                {
                    return &shape;
                }
            }

            return nullptr;
        }

        void writeViolationEdges(std::ofstream& output, const domain::Violation& violation, const std::vector<domain::Shape>& shapes, const geometry::BoundingBox& bounds)
        {
            if (!violation.getMarker().has_value())
            {
                return;
            }

            const auto& marker = violation.getMarker().value();
            const auto& shapeIds = violation.getShapeIds();

            if (shapeIds.empty())
            {
                return;
            }

            const domain::Shape* firstShape = findShapeById(shapes, shapeIds[0]);

            if (firstShape == nullptr)
            {
                return;
            }

            const auto firstShapeEdges = firstShape->getPolygon().getEdges();

            if (marker.firstEdgeIndex.has_value())
            {
                const std::size_t edgeIndex = marker.firstEdgeIndex.value();

                if (edgeIndex < firstShapeEdges.size())
                {
                    writeHighlightedEdge(output, firstShapeEdges[edgeIndex], bounds);
                }
            }

            // MinWidth:
            // Both offending edges belong to the same Shape.
            if (shapeIds.size() == 1)
            {
                if (marker.secondEdgeIndex.has_value())
                {
                    const std::size_t edgeIndex = marker.secondEdgeIndex.value();

                    if (edgeIndex < firstShapeEdges.size())
                    {
                        writeHighlightedEdge(output, firstShapeEdges[edgeIndex], bounds);
                    }
                }

                return;
            }

            // Two-shape violations such as spacing/enclosure.
            const domain::Shape* secondShape = findShapeById(shapes, shapeIds[1]);

            if (secondShape == nullptr)
            {
                return;
            }

            const auto secondShapeEdges = secondShape->getPolygon().getEdges();

            if (marker.secondEdgeIndex.has_value())
            {
                const std::size_t edgeIndex = marker.secondEdgeIndex.value();

                if (edgeIndex < secondShapeEdges.size())
                {
                    writeHighlightedEdge(output, secondShapeEdges[edgeIndex], bounds);
                }
            }
        }

        void writePolygon(std::ofstream& output, const domain::Shape& shape, const geometry::BoundingBox& bounds)
        {
            output << "  <polygon points=\"";

            for (const auto& point : shape.getPolygon().getVertices())
            {
                output << toSvgX(point.getX(), bounds) << "," << toSvgY(point.getY(), bounds) << " ";
            }

            output
                << "\" "
                << "fill=\""
                << layerFillColor(shape.getLayer())
                << "\" "
                << "fill-opacity=\"0.35\" "
                << "stroke=\""
                << layerStrokeColor(shape.getLayer())
                << "\" "
                << "stroke-width=\""
                << SHAPE_STROKE_WIDTH
                << "\""
                << " />\n";
        }

        void writeShapeId(std::ofstream& output, const domain::Shape& shape, const geometry::BoundingBox& bounds)
        {
            const auto shapeBounds = shape.getPolygon().getBoundingBox();

            const double x = toSvgX(shapeBounds.getMinX(), bounds) + 2.0;
            const double y = toSvgY(shapeBounds.getMaxY(), bounds) + 10.0;

            output
                << "  <text "
                << "x=\"" << x << "\" "
                << "y=\"" << y << "\" "
                << "font-size=\""
                << SHAPE_ID_FONT_SIZE
                << "\" "
                << "fill=\"black\">"
                << shape.getId()
                << "</text>\n";
        }

        void writeViolationMarker(std::ofstream& output, const domain::Violation& violation, const geometry::BoundingBox& bounds)
        {
            if (!violation.getMarker().has_value())
            {
                return;
            }

            const auto& marker = violation.getMarker().value();

            if (!marker.firstPoint.has_value() || !marker.secondPoint.has_value())
            {
                return;
            }

            const geometry::Point& firstPoint = marker.firstPoint.value();
            const geometry::Point& secondPoint = marker.secondPoint.value();

            const double x1 = toSvgX(firstPoint.getX(), bounds);
            const double y1 = toSvgY(firstPoint.getY(), bounds);
            const double x2 = toSvgX(secondPoint.getX(), bounds);
            const double y2 = toSvgY(secondPoint.getY(), bounds);

            output << "  <g>\n";

            output
                << "    <line "
                << "class=\"violation-marker\" "
                << "x1=\"" << x1 << "\" "
                << "y1=\"" << y1 << "\" "
                << "x2=\"" << x2 << "\" "
                << "y2=\"" << y2 << "\" "
                << "stroke=\"red\" "
                << "stroke-width=\""
                << VIOLATION_MARKER_WIDTH
                << "\" "
                << "stroke-linecap=\"round\""
                << " />\n";

            // First witness point.
            output
                << "    <circle "
                << "class=\"violation-point\" "
                << "cx=\"" << x1 << "\" "
                << "cy=\"" << y1 << "\" "
                << "r=\""
                << VIOLATION_POINT_RADIUS
                << "\" "
                << "fill=\"red\""
                << " />\n";

            // Second witness point.
            output
                << "    <circle "
                << "class=\"violation-point\" "
                << "cx=\"" << x2 << "\" "
                << "cy=\"" << y2 << "\" "
                << "r=\""
                << VIOLATION_POINT_RADIUS
                << "\" "
                << "fill=\"red\""
                << " />\n";

            const double midpointX = (x1 + x2) / 2.0;
            const double midpointY = (y1 + y2) / 2.0;

            output
                << "    <text "
                << "class=\"violation-label\" "
                << "x=\"" << midpointX << "\" "
                << "y=\"" << midpointY - 1.0 << "\" "
                << "font-size=\""
                << VIOLATION_LABEL_FONT_SIZE
                << "\" "
                << "text-anchor=\"middle\" "
                << "fill=\"red\">"
                << violation.getTypeAsString()
                << " "
                << violation.getActualValue()
                << "/"
                << violation.getRequiredValue()
                << "</text>\n";

            output << "  </g>\n";
        }

        void writeViolationRegion(std::ofstream& output, const domain::Violation& violation, const geometry::BoundingBox& bounds)
        {
            if (!violation.getMarker().has_value())
            {
                return;
            }

            const auto& marker = violation.getMarker().value();

            if (!marker.region.has_value())
            {
                return;
            }

            const auto& region = marker.region.value();

            const double x = toSvgX(region.getMinX(), bounds);
            const double y = toSvgY(region.getMaxY(), bounds);
            const double width = (region.getMaxX() - region.getMinX()) * SVG_SCALE;
            const double height = (region.getMaxY() - region.getMinY()) * SVG_SCALE;

            output << "  <g>\n";

            output
                << "    <rect "
                << "class=\"violation-region\" "
                << "x=\"" << x << "\" "
                << "y=\"" << y << "\" "
                << "width=\"" << width << "\" "
                << "height=\"" << height << "\" "
                << "fill=\"red\" "
                << "fill-opacity=\"0.08\" "
                << "stroke=\"red\" "
                << "stroke-width=\""
                << VIOLATION_MARKER_WIDTH
                << "\" "
                << "/>\n";

            const double labelX = x + width / 2.0;
            const double labelY = y + VIOLATION_LABEL_FONT_SIZE + 2.0;

            output
                << "    <text "
                << "class=\"violation-label\" "
                << "x=\"" << labelX << "\" "
                << "y=\"" << labelY << "\" "
                << "font-size=\""
                << VIOLATION_LABEL_FONT_SIZE
                << "\" "
                << "text-anchor=\"middle\" "
                << "fill=\"red\">"
                << violation.getTypeAsString()
                << " "
                << violation.getActualValue()
                << "/"
                << violation.getRequiredValue()
                << "</text>\n";

            output << "  </g>\n";
        }

    }

    void SVGReportWriter::write(const std::vector<domain::Shape>& shapes, const std::vector<domain::Violation>& violations, const std::string& filePath)
    {
        std::ofstream output(filePath);

        if (!output.is_open())
        {
            throw std::runtime_error("Failed to open SVG output file");
        }

        const auto reportBounds = calculateReportBounds(shapes, violations);

        if (!reportBounds.has_value())
        {
            output
                << "<svg "
                << "xmlns=\"http://www.w3.org/2000/svg\" "
                << "width=\"100\" "
                << "height=\"100\" "
                << "viewBox=\"0 0 100 100\">\n";

            output << "</svg>\n";

            return;
        }

        const geometry::BoundingBox& bounds = reportBounds.value();

        const double svgWidth = (bounds.getMaxX() - bounds.getMinX()) * SVG_SCALE + 2.0 * SVG_PADDING;
        const double svgHeight = (bounds.getMaxY() - bounds.getMinY()) * SVG_SCALE + 2.0 * SVG_PADDING;

        output
            << "<svg "
            << "xmlns=\"http://www.w3.org/2000/svg\" "
            << "width=\"" << svgWidth << "\" "
            << "height=\"" << svgHeight << "\" "
            << "viewBox=\"0 0 "
            << svgWidth << " "
            << svgHeight << "\">\n";

        output << "  <g id=\"layout\">\n";

        for (const auto& shape : shapes)
        {
            writePolygon(output, shape, bounds);
        }

        output << "  </g>\n";

        output << "  <g id=\"shape-labels\">\n";

        for (const auto& shape : shapes)
        {
            writeShapeId(output, shape, bounds);
        }

        output << "  </g>\n";

        output << "  <g id=\"violations\">\n";

        for (const auto& violation : violations)
        {
            writeViolationEdges(output, violation, shapes, bounds);
            writeViolationMarker(output, violation, bounds);
            writeViolationRegion(output, violation, bounds);
        }

        output << "  </g>\n";

        output << "</svg>\n";
    }

}
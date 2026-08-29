#include "drcheck/io/SVGReportWriter.h"

#include <fstream>
#include <stdexcept>
#include <optional>
#include <algorithm>

namespace drcheck::io {
    namespace {

        constexpr double SVG_WIDTH = 1200.0;
        constexpr double SVG_HEIGHT = 800.0;
        constexpr double SVG_PADDING = 40.0;

        constexpr double SHAPE_STROKE_WIDTH = 2.0;
        constexpr double VIOLATION_EDGE_WIDTH = 3.0;
        constexpr double VIOLATION_MARKER_WIDTH = 2.0;
        constexpr double VIOLATION_POINT_RADIUS = 4.0;

        constexpr double VIOLATION_LABEL_FONT_SIZE = 14.0;

        constexpr double LEGEND_WIDTH = 170.0;
        constexpr double LEGEND_ITEM_HEIGHT = 30.0;
        constexpr double LEGEND_COLOR_SIZE = 18.0;
        constexpr double LEGEND_FONT_SIZE = 14.0;

        // Converts layout coordinates to SVG coordinates while fitting the
        // complete report inside a fixed SVG viewport.
        class SvgTransform
        {
        public:
            explicit SvgTransform(const geometry::BoundingBox& bounds)
                : bounds(bounds)
            {
                const double layoutWidth = bounds.getMaxX() - bounds.getMinX();
                const double layoutHeight = bounds.getMaxY() - bounds.getMinY();
                const double availableWidth = SVG_WIDTH - 2.0 * SVG_PADDING - LEGEND_WIDTH;
                const double availableHeight = SVG_HEIGHT - 2.0 * SVG_PADDING;
                scale = std::min(availableWidth / layoutWidth, availableHeight / layoutHeight);

                const double scaledWidth = layoutWidth * scale;
                const double scaledHeight = layoutHeight * scale;

                // Center the layout inside the viewport.
                offsetX = SVG_PADDING + (availableWidth - scaledWidth) / 2.0;
                offsetY = SVG_PADDING + (availableHeight - scaledHeight) / 2.0;
            }

            double toSvgX(double layoutX) const
            {
                return (layoutX - bounds.getMinX()) * scale + offsetX;
            }

            double toSvgY(double layoutY) const
            {
                // SVG Y increases downward, so the layout Y axis is flipped.
                return (bounds.getMaxY() - layoutY) * scale + offsetY;
            }

            double scaleLength(double length) const
            {
                return length * scale;
            }

        private:
            geometry::BoundingBox bounds;

            double scale;
            double offsetX;
            double offsetY;
        };

        double roundForReport(double value)
        {
            constexpr double scale = 1'000.0;
            return std::round(value * scale) / scale;
        }

        int getLayerDrawPriority(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::NW:
                return 0;
            
            case domain::Layer::VTL_N:
                return 1;

            case domain::Layer::VTL_P:
                return 2;

            case domain::Layer::PDK:
                return 3;

            case domain::Layer::PO:
                return 4;

            case domain::Layer::NP:
                return 5;

            case domain::Layer::PP:
                return 6;

            case domain::Layer::OD:
                return 7;

            case domain::Layer::CO:
                return 8;

            case domain::Layer::M1:
                return 9;

            case domain::Layer::M1_PIN:
                return 10;

            case domain::Layer::VIA1:
                return 11;

            case domain::Layer::M2:
                return 12;

            case domain::Layer::M2_PIN:
                return 13;
            }

            return 100;
        }

        std::vector<domain::Shape> getShapesInDrawOrder(const std::vector<domain::Shape>& shapes)
        {
            std::vector<domain::Shape> orderedShapes = shapes;

            std::stable_sort(
                orderedShapes.begin(),
                orderedShapes.end(),
                [](const domain::Shape& first, const domain::Shape& second)
                {
                    return getLayerDrawPriority(first.getLayer()) < getLayerDrawPriority(second.getLayer());
                });

            return orderedShapes;
        }

        std::string layerToString(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::NW:
                return "NW";

            case domain::Layer::NP:
                return "NP";

            case domain::Layer::PP:
                return "PP";

            case domain::Layer::M1:
                return "M1";

            case domain::Layer::M1_PIN:
                return "M1_PIN";

            case domain::Layer::M2:
                return "M2";

            case domain::Layer::M2_PIN:
                return "M2_PIN";

            case domain::Layer::VIA1:
                return "VIA1";

            case domain::Layer::PO:
                return "PO";

            case domain::Layer::OD:
                return "OD";

            case domain::Layer::CO:
                return "CO";

            case domain::Layer::PDK:
                return "PDK";

            case domain::Layer::VTL_N:
                return "VTL_N";

            case domain::Layer::VTL_P:
                return "VTL_P";
            }


            return "Unknown";
        }

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

        std::string layerFillColor(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::NW:
                return "cream";

            case domain::Layer::NP:
                return "gray";

            case domain::Layer::PP:
                return "magenta";

            case domain::Layer::M1:
                return "cyan";

            case domain::Layer::M1_PIN:
                return "cyan";

            case domain::Layer::M2:
                return "gold";

            case domain::Layer::M2_PIN:
                return "gold";

            case domain::Layer::VIA1:
                return "yellow";

            case domain::Layer::PO:
                return "blue";

            case domain::Layer::OD:
                return "red";

            case domain::Layer::CO:
                return "green";

            case domain::Layer::PDK:
                return "purple";

            case domain::Layer::VTL_N:
                return "blue";

            case domain::Layer::VTL_P:
                return "gray";
            }

            return "lightgray";
        }

        std::string layerStrokeColor(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::NW:
                return "cream";

            case domain::Layer::NP:
                return "gray";

            case domain::Layer::PP:
                return "magenta";

            case domain::Layer::M1:
                return "cyan";

            case domain::Layer::M1_PIN:
                return "cyan";

            case domain::Layer::M2:
                return "gold";

            case domain::Layer::M2_PIN:
                return "gold";

            case domain::Layer::VIA1:
                return "yellow";

            case domain::Layer::PO:
                return "blue";

            case domain::Layer::OD:
                return "red";

            case domain::Layer::CO:
                return "green";

            case domain::Layer::PDK:
                return "purple";

            case domain::Layer::VTL_N:
                return "blue";

            case domain::Layer::VTL_P:
                return "gray";
            }

            return "black";
        }

        void writeHighlightedEdge(std::ofstream& output, const geometry::Segment& edge, const SvgTransform& transform)
        {
            const double x1 =
                transform.toSvgX(edge.getStart().getX());

            const double y1 =
                transform.toSvgY(edge.getStart().getY());

            const double x2 =
                transform.toSvgX(edge.getEnd().getX());

            const double y2 =
                transform.toSvgY(edge.getEnd().getY());

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

        std::string getViolationLayerClasses(const domain::Violation& violation, const std::vector<domain::Shape>& shapes)
        {
            std::vector<std::string> classes;

            for (std::size_t shapeId : violation.getShapeIds())
            {
                const domain::Shape* shape = findShapeById(shapes, shapeId);

                if (shape == nullptr)
                {
                    continue;
                }

                const std::string className = "layer-" + layerToString(shape->getLayer());

                bool alreadyExists = false;

                for (const auto& existingClass : classes)
                {
                    if (existingClass == className)
                    {
                        alreadyExists = true;
                        break;
                    }
                }

                if (!alreadyExists)
                {
                    classes.push_back(className);
                }
            }

            std::string result;

            for (std::size_t i = 0; i < classes.size(); ++i)
            {
                if (i > 0)
                {
                    result += " ";
                }

                result += classes[i];
            }

            return result;
        }

        void writeViolationEdges(std::ofstream& output, const domain::Violation& violation, const std::vector<domain::Shape>& shapes, const SvgTransform& transform)
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
                    writeHighlightedEdge(output, firstShapeEdges[edgeIndex], transform
                    );
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
                        writeHighlightedEdge(output, firstShapeEdges[edgeIndex], transform);
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
                    writeHighlightedEdge(output, secondShapeEdges[edgeIndex], transform);
                }
            }
        }

        void writePolygon(std::ofstream& output, const domain::Shape& shape, const SvgTransform& transform)
        {
            output
                << "  <g class=\"shape layer-"
                << layerToString(shape.getLayer())
                << "\">\n";

            output
                << "    <title>"
                << "Shape ID: " << shape.getId()
                << "&#10;Layer: " << layerToString(shape.getLayer())
                << "</title>\n";

            output << "  <polygon points=\"";

            for (const auto& point : shape.getPolygon().getVertices())
            {
                output
                    << transform.toSvgX(point.getX())
                    << ","
                    << transform.toSvgY(point.getY())
                    << " ";
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

            if (shape.getLayer() == domain::Layer::M1_PIN || shape.getLayer() == domain::Layer::M2_PIN) {

                const auto bounds = shape.getPolygon().getBoundingBox();

                const double minX = transform.toSvgX(bounds.getMinX());
                const double maxX = transform.toSvgX(bounds.getMaxX());

                const double topY = transform.toSvgY(bounds.getMaxY());
                const double bottomY = transform.toSvgY(bounds.getMinY());

                output
                    << "      <line "
                    << "x1=\"" << minX << "\" "
                    << "y1=\"" << topY << "\" "
                    << "x2=\"" << maxX << "\" "
                    << "y2=\"" << bottomY << "\" "
                    << "stroke=\"lightgray\" "
                    << "stroke-width=\"1.5\""
                    << " />\n"

                    << "      <line "
                    << "x1=\"" << minX << "\" "
                    << "y1=\"" << bottomY << "\" "
                    << "x2=\"" << maxX << "\" "
                    << "y2=\"" << topY << "\" "
                    << "stroke=\"lightgray\" "
                    << "stroke-width=\"1.5\""
                    << " />\n";

            }
            output << "  </g>\n";
        }

        void writeViolationMarker(std::ofstream& output, const domain::Violation& violation, const SvgTransform& transform)
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

            const double x1 = transform.toSvgX(firstPoint.getX());
            const double y1 = transform.toSvgY(firstPoint.getY());
            const double x2 = transform.toSvgX(secondPoint.getX());
            const double y2 = transform.toSvgY(secondPoint.getY());



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
                << "y=\"" << midpointY - VIOLATION_LABEL_FONT_SIZE / 2.0 << "\" "
                << "font-size=\"" << VIOLATION_LABEL_FONT_SIZE << "\" "
                << "text-anchor=\"middle\" "
                << "fill=\"red\">";

            output
                << "<tspan x=\"" << midpointX << "\">"
                << violation.getTypeAsString();

            if (marker.firstLayer.has_value())
            {
                output
                    << " "
                    << layerToString(marker.firstLayer.value());

                if (marker.secondLayer.has_value())
                {
                    output
                        << " With "
                        << layerToString(marker.secondLayer.value());
                }
            }

            output
                << "</tspan>";

            output
                << "<tspan "
                << "x=\"" << midpointX << "\" "
                << "dy=\"" << VIOLATION_LABEL_FONT_SIZE + 2.0 << "\">"
                << roundForReport(violation.getActualValue())
                << "/"
                << violation.getRequiredValue()
                << "</tspan>";

            output << "</text>\n";

            output << "  </g>\n";
        }

        void writeViolationRegion(
            std::ofstream& output,
            const domain::Violation& violation,
            const SvgTransform& transform)
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
            const double x = transform.toSvgX(region.getMinX());
            const double y = transform.toSvgY(region.getMaxY());
            const double width = transform.scaleLength(region.getMaxX() - region.getMinX());
            const double height = transform.scaleLength(region.getMaxY() - region.getMinY()
                );

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

            const double labelY = y + VIOLATION_LABEL_FONT_SIZE + 4.0;

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
    void writeLegendItem(std::ofstream& output, domain::Layer layer, double x, double y)
    {
        const std::string className = "layer-" + layerToString(layer);

        output
            << "    <g "
            << "class=\"legend-item\" "
            << "data-layer=\"" << className << "\" "
            << "onclick=\"toggleLayer(this)\" "
            << "style=\"cursor: pointer;\">\n";

        output
            << "    <rect "
            << "x=\"" << x << "\" "
            << "y=\"" << y << "\" "
            << "width=\"" << LEGEND_COLOR_SIZE << "\" "
            << "height=\"" << LEGEND_COLOR_SIZE << "\" "
            << "fill=\"" << layerFillColor(layer) << "\" "
            << "fill-opacity=\"0.35\" "
            << "stroke=\"" << layerStrokeColor(layer) << "\" "
            << "stroke-width=\"2\""
            << " />\n";

output
    << "      <text "
    << "class=\"legend-label\" "
    << "x=\"" << x + LEGEND_COLOR_SIZE + 10.0 << "\" "
    << "y=\"" << y + LEGEND_COLOR_SIZE - 3.0 << "\" "
    << "font-size=\"" << LEGEND_FONT_SIZE << "\" "
    << "fill=\"black\">"
    << layerToString(layer)
    << "</text>\n";

        output << "    </g>\n";
    }

    std::vector<domain::Layer> getExistingLayers(const std::vector<domain::Shape>& shapes)
    {
        std::vector<domain::Layer> layers;

        for (const auto& shape : shapes)
        {
            const domain::Layer layer = shape.getLayer();

            if (std::find(layers.begin(), layers.end(), layer) == layers.end())
            {
                layers.push_back(layer);
            }
        }

        return layers;
    }

    void writeLegend(std::ofstream& output, const std::vector<domain::Layer>& layers, std::size_t violationCount)
    {
        if (layers.empty())
        {
            return;
        }

        const double x = SVG_WIDTH - LEGEND_WIDTH + 20.0;
        const double y = SVG_PADDING;

        output << "  <g id=\"layer-legend\">\n";

        output
            << "    <text "
            << "x=\"" << x << "\" "
            << "y=\"" << y << "\" "
            << "font-size=\"20\" "
            << "font-weight=\"bold\" "
            << "fill=\"black\">"
            << "Violations: "
            << violationCount
            << "</text>\n";

        output
            << "    <text "
            << "x=\"" << x << "\" "
            << "y=\"" << y + 25.0 << "\" "
            << "font-size=\"18\" "
            << "font-weight=\"bold\" "
            << "fill=\"black\">"
            << "Layers"
            << "</text>\n" 

            << "    <text "
            << "id=\"toggle-all-layers\" "
            << "x=\"" << x + 65.0 << "\" "
            << "y=\"" << y + 25.0 << "\" "
            << "font-size=\"13\" "
            << "fill=\"#555555\" "
            << "text-decoration=\"underline\" "
            << "style=\"cursor: pointer;\" "
            << "onclick=\"toggleAllLayers()\">"
            << "Hide All"
            << "</text>\n";



        double itemY = y + 50.0;

        for (const domain::Layer layer : layers)
        {
            writeLegendItem(output, layer, x, itemY);
            itemY += LEGEND_ITEM_HEIGHT;
        }

        output << "  </g>\n";
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
                << "width=\"100%\" "
                << "height=\"90vh\" "
                << "viewBox=\"0 0 "
                << SVG_WIDTH << " "
                << SVG_HEIGHT << "\" "
                << "preserveAspectRatio=\"xMidYMid meet\">\n"
                << "</svg>\n";
            return;
        }

        const geometry::BoundingBox& bounds = reportBounds.value();

        const SvgTransform transform(bounds);

        output
            << "<svg "
            << "xmlns=\"http://www.w3.org/2000/svg\" "
            << "width=\"100%\" "
            << "height=\"90vh\" "
            << "viewBox=\"0 0 "
            << SVG_WIDTH << " "
            << SVG_HEIGHT << "\" "
            << "preserveAspectRatio=\"xMidYMid meet\">\n";

        output
            << "  <style>\n"
            << "    .violation-label {\n"
            << "      visibility: hidden;\n"
            << "      pointer-events: none;\n"
            << "    }\n"
            << "    .violation:hover .violation-label {\n"
            << "      visibility: visible;\n"
            << "    }\n"
            << "    .violation {\n"
            << "      cursor: pointer;\n"
            << "    }\n"
            << "    .legend-item.disabled .legend-label {\n"
            << "      fill: darkgray;\n"
            << "    }\n"
            << "    .legend-item.disabled rect {\n"
            << "      opacity: 0.35;\n"
            << "    }\n"
            << "  </style>\n";

        output << "  <g id=\"layout\">\n";

        const auto orderedShapes = getShapesInDrawOrder(shapes);

        for (const auto& shape : orderedShapes)
        {
            writePolygon(output, shape, transform);
        }

        output << "  </g>\n";

        output << "  <g id=\"violations\">\n";

        for (const auto& violation : violations)
        {
            output
                << "    <g class=\"violation\" "
                << "data-layers=\""
                << getViolationLayerClasses(violation, shapes)
                << "\">\n";

            writeViolationEdges(output, violation, shapes, transform);
            writeViolationMarker(output, violation, transform);
            writeViolationRegion(output, violation, transform);

            output << "    </g>\n";
        }

        output << "  </g>\n";

        const auto existingLayers = getExistingLayers(shapes);
        writeLegend(output, existingLayers, violations.size());

        output
            << "  <script><![CDATA[\n"
            << "    function updateLayerVisibility() {\n"
            << "      const hiddenLayers = new Set();\n"
            << "\n"
            << "      document.querySelectorAll('.legend-item.disabled').forEach(function(item) {\n"
            << "        hiddenLayers.add(item.dataset.layer);\n"
            << "      });\n"
            << "\n"
            << "      document.querySelectorAll('.shape').forEach(function(shape) {\n"
            << "        let hideShape = false;\n"
            << "\n"
            << "        shape.classList.forEach(function(className) {\n"
            << "          if (className.startsWith('layer-') && hiddenLayers.has(className)) {\n"
            << "            hideShape = true;\n"
            << "          }\n"
            << "        });\n"
            << "\n"
            << "        shape.style.display = hideShape ? 'none' : '';\n"
            << "      });\n"
            << "\n"
            << "      document.querySelectorAll('.violation').forEach(function(violation) {\n"
            << "        const layersText = violation.dataset.layers || '';\n"
            << "        const layers = layersText === '' ? [] : layersText.split(' ');\n"
            << "\n"
            << "        let hideViolation = false;\n"
            << "\n"
            << "        layers.forEach(function(layer) {\n"
            << "          if (hiddenLayers.has(layer)) {\n"
            << "            hideViolation = true;\n"
            << "          }\n"
            << "        });\n"
            << "\n"
            << "        violation.style.display = hideViolation ? 'none' : '';\n"
            << "      });\n"
            << "    }\n"
            << "\n"
            << "    function updateToggleAllText() {\n"
            << "      const legendItems = document.querySelectorAll('.legend-item');\n"
            << "      const toggleText = document.getElementById('toggle-all-layers');\n"
            << "\n"
            << "      if (!toggleText || legendItems.length === 0) {\n"
            << "        return;\n"
            << "      }\n"
            << "\n"
            << "      const allHidden = Array.from(legendItems).every(function(item) {\n"
            << "        return item.classList.contains('disabled');\n"
            << "      });\n"
            << "\n"
            << "      toggleText.textContent = allHidden ? 'Show All' : 'Hide All';\n"
            << "    }\n"
            << "\n"
            << "    function toggleAllLayers() {\n"
            << "      const legendItems = document.querySelectorAll('.legend-item');\n"
            << "\n"
            << "      const allHidden = Array.from(legendItems).every(function(item) {\n"
            << "        return item.classList.contains('disabled');\n"
            << "      });\n"
            << "\n"
            << "      legendItems.forEach(function(item) {\n"
            << "        if (allHidden) {\n"
            << "          item.classList.remove('disabled');\n"
            << "        }\n"
            << "        else {\n"
            << "          item.classList.add('disabled');\n"
            << "        }\n"
            << "      });\n"
            << "\n"
            << "      updateLayerVisibility();\n"
            << "      updateToggleAllText();\n"
            << "    }\n"
            << "\n"
            << "    function toggleLayer(legendItem) {\n"
            << "      legendItem.classList.toggle('disabled');\n"
            << "      updateLayerVisibility();\n"
            << "      updateToggleAllText();\n"
            << "    }\n"
            << "\n"
            << "    const svg = document.documentElement;\n"
            << "\n"
            << "    const originalWidth = " << SVG_WIDTH << ";\n"
            << "    const originalHeight = " << SVG_HEIGHT << ";\n"
            << "\n"
            << "    let viewX = 0;\n"
            << "    let viewY = 0;\n"
            << "    let viewWidth = originalWidth;\n"
            << "    let viewHeight = originalHeight;\n"
            << "\n"
            << "    const MIN_ZOOM_FACTOR = 0.1;\n"
            << "\n"
            << "    svg.addEventListener('wheel', function(event) {\n"
            << "      event.preventDefault();\n"
            << "\n"
            << "      const rect = svg.getBoundingClientRect();\n"
            << "\n"
            << "      const mouseX = viewX +\n"
            << "        ((event.clientX - rect.left) / rect.width) * viewWidth;\n"
            << "\n"
            << "      const mouseY = viewY +\n"
            << "        ((event.clientY - rect.top) / rect.height) * viewHeight;\n"
            << "\n"
            << "      let zoomFactor = event.deltaY < 0 ? 0.9 : 1.1;\n"
            << "\n"
            << "      let newWidth = viewWidth * zoomFactor;\n"
            << "\n"
            << "      const minWidth = originalWidth * MIN_ZOOM_FACTOR;\n"
            << "\n"
            << "      if (newWidth < minWidth) {\n"
            << "        newWidth = minWidth;\n"
            << "      }\n"
            << "\n"
            << "      if (newWidth > originalWidth) {\n"
            << "        newWidth = originalWidth;\n"
            << "      }\n"
            << "\n"
            << "      zoomFactor = newWidth / viewWidth;\n"
            << "\n"
            << "      const newHeight = viewHeight * zoomFactor;\n"
            << "\n"
            << "      viewX = mouseX - (mouseX - viewX) * zoomFactor;\n"
            << "      viewY = mouseY - (mouseY - viewY) * zoomFactor;\n"
            << "\n"
            << "      viewWidth = newWidth;\n"
            << "      viewHeight = newHeight;\n"
            << "\n"
            << "      if (viewWidth == originalWidth) {\n"
            << "        viewX = 0;\n"
            << "        viewY = 0;\n"
            << "        viewHeight = originalHeight;\n"
            << "      }\n"
            << "\n"
            << "      svg.setAttribute(\n"
            << "        'viewBox',\n"
            << "        viewX + ' ' + viewY + ' ' + viewWidth + ' ' + viewHeight\n"
            << "      );\n"
            << "    }, { passive: false });\n"
            << "\n"
            << "    function resetView() {\n"
            << "      viewX = 0;\n"
            << "      viewY = 0;\n"
            << "      viewWidth = originalWidth;\n"
            << "      viewHeight = originalHeight;\n"
            << "\n"
            << "      svg.setAttribute(\n"
            << "        'viewBox',\n"
            << "        '0 0 ' + originalWidth + ' ' + originalHeight\n"
            << "      );\n"
            << "    }\n"
            << "\n"
            << "    document.addEventListener('keydown', function(event) {\n"
            << "      if (event.key === 'f' || event.key === 'F') {\n"
            << "        resetView();\n"
            << "      }\n"
            << "    });\n"
            << "  ]]></script>\n";

        output << "</svg>\n";
    }

}
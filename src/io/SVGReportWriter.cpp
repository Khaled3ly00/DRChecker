#include "drcheck/io/SVGReportWriter.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>

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

        constexpr double VIOLATION_PANEL_WIDTH = 320.0;
        constexpr double VIOLATION_ITEM_HEIGHT = 28.0;
        constexpr double PANEL_TITLE_FONT_SIZE = 20.0;
        constexpr double PANEL_TEXT_FONT_SIZE = 12.0;
        constexpr double PANEL_BUTTON_WIDTH = 80.0;
        constexpr double PANEL_BUTTON_HEIGHT = 24.0;
        constexpr std::size_t VIOLATIONS_PER_PAGE = 19;

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
                const double availableWidth = SVG_WIDTH - 2.0 * SVG_PADDING - LEGEND_WIDTH - VIOLATION_PANEL_WIDTH;
                const double availableHeight = SVG_HEIGHT - 2.0 * SVG_PADDING;
                scale = std::min(availableWidth / layoutWidth, availableHeight / layoutHeight);

                const double scaledWidth = layoutWidth * scale;
                const double scaledHeight = layoutHeight * scale;

                // Center the layout inside the viewport.
                offsetX = SVG_PADDING + VIOLATION_PANEL_WIDTH + (availableWidth - scaledWidth) / 2.0;
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
            constexpr double scale = 10'000.0;
            return std::round(value * scale) / scale;
        }

        std::size_t getViolationPageCount(std::size_t violationCount)
        {
            // Keep an empty report on a valid first page so the navigation
            // indicator never displays the nonsensical value "1 / 0".
            return violationCount == 0
                ? 1
                : 1 + (violationCount - 1) / VIOLATIONS_PER_PAGE;
        }

        struct LayerStyle
        {
            const char* name;
            const char* fillColor;
            const char* strokeColor;
            int drawPriority;
        };

        // Keep all SVG presentation data for a layer in one place so drawing,
        // labels, and the legend cannot silently drift apart.
        constexpr LayerStyle getLayerStyle(domain::Layer layer)
        {
            switch (layer)
            {
            case domain::Layer::NW:
                return { "NW", "cream", "cream", 0 };

            case domain::Layer::VTL_N:
                return { "VTL_N", "blue", "blue", 1 };

            case domain::Layer::VTL_P:
                return { "VTL_P", "gray", "gray", 2 };

            case domain::Layer::PDK:
                return { "PDK", "purple", "purple", 3 };

            case domain::Layer::PO:
                return { "PO", "blue", "blue", 4 };

            case domain::Layer::NP:
                return { "NP", "gray", "gray", 5 };

            case domain::Layer::PP:
                return { "PP", "magenta", "magenta", 6 };

            case domain::Layer::OD:
                return { "OD", "red", "red", 7 };

            case domain::Layer::CO:
                return { "CO", "green", "green", 8 };

            case domain::Layer::M1:
                return { "M1", "cyan", "cyan", 9 };

            case domain::Layer::M1_PIN:
                return { "M1_PIN", "cyan", "cyan", 10 };

            case domain::Layer::VIA1:
                return { "VIA1", "yellow", "yellow", 11 };

            case domain::Layer::M2:
                return { "M2", "gold", "gold", 12 };

            case domain::Layer::M2_PIN:
                return { "M2_PIN", "gold", "gold", 13 };
            }

            return { "Unknown", "lightgray", "black", 100 };
        }

        std::vector<domain::Shape> getShapesInDrawOrder(const std::vector<domain::Shape>& shapes)
        {
            std::vector<domain::Shape> orderedShapes = shapes;

            std::stable_sort(
                orderedShapes.begin(),
                orderedShapes.end(),
                [](const domain::Shape& first, const domain::Shape& second)
                {
                    return getLayerStyle(first.getLayer()).drawPriority < getLayerStyle(second.getLayer()).drawPriority;
                });

            return orderedShapes;
        }

        void mergeInto(std::optional<geometry::BoundingBox>& bounds, const geometry::BoundingBox& addition)
        {
            bounds = bounds ? bounds->mergedWith(addition) : addition;
        }

        // Violation regions can extend beyond every shape, so they must take
        // part in fitting the report to the viewport.
        std::optional<geometry::BoundingBox> calculateReportBounds(const std::vector<domain::Shape>& shapes, const std::vector<domain::Violation>& violations)
        {
            std::optional<geometry::BoundingBox> bounds;

            for (const auto& shape : shapes)
            {
                mergeInto(bounds, shape.getPolygon().getBoundingBox());
            }

            for (const auto& violation : violations)
            {
                const auto& marker = violation.getMarker();

                if (!marker || !marker->region)
                {
                    continue;
                }

                mergeInto(bounds, *marker->region);
            }

            return bounds;
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
            const auto match = std::find_if(
                shapes.begin(),
                shapes.end(),
                [id](const domain::Shape& shape) { return shape.getId() == id; });

            return match == shapes.end() ? nullptr : &*match;
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

                std::string className = "layer-";
                className += getLayerStyle(shape->getLayer()).name;

                if (std::find(classes.begin(), classes.end(), className) == classes.end())
                {
                    classes.push_back(className);
                }
            }

            std::ostringstream result;

            for (std::size_t i = 0; i < classes.size(); ++i)
            {
                if (i > 0)
                {
                    result << ' ';
                }

                result << classes[i];
            }

            return result.str();
        }

        void writeHighlightedEdgeAtIndex(
            std::ofstream& output,
            const std::vector<geometry::Segment>& edges,
            const std::optional<std::size_t>& edgeIndex,
            const SvgTransform& transform)
        {
            if (edgeIndex && *edgeIndex < edges.size())
            {
                writeHighlightedEdge(output, edges[*edgeIndex], transform);
            }
        }

        void writeViolationEdges(std::ofstream& output, const domain::Violation& violation, const std::vector<domain::Shape>& shapes, const SvgTransform& transform)
        {
            const auto& marker = violation.getMarker();

            if (!marker)
            {
                return;
            }

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

            writeHighlightedEdgeAtIndex(output, firstShapeEdges, marker->firstEdgeIndex, transform);

            // Width violations reference two edges on one shape; spacing and
            // enclosure violations reference one edge on each of two shapes.
            if (shapeIds.size() == 1)
            {
                writeHighlightedEdgeAtIndex(output, firstShapeEdges, marker->secondEdgeIndex, transform);
                return;
            }

            const domain::Shape* secondShape = findShapeById(shapes, shapeIds[1]);

            if (secondShape == nullptr)
            {
                return;
            }

            const auto secondShapeEdges = secondShape->getPolygon().getEdges();
            writeHighlightedEdgeAtIndex(output, secondShapeEdges, marker->secondEdgeIndex, transform);
        }

        void writePinCross(std::ofstream& output, const domain::Shape& shape, const SvgTransform& transform)
        {
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

        void writePolygon(std::ofstream& output, const domain::Shape& shape, const SvgTransform& transform)
        {
            const LayerStyle layerStyle = getLayerStyle(shape.getLayer());

            output
                << "  <g class=\"shape layer-"
                << layerStyle.name
                << "\">\n";

            output
                << "    <title>"
                << "Shape ID: " << shape.getId()
                << "&#10;Layer: " << layerStyle.name
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
                << layerStyle.fillColor
                << "\" "
                << "fill-opacity=\"0.35\" "
                << "stroke=\""
                << layerStyle.strokeColor
                << "\" "
                << "stroke-width=\""
                << SHAPE_STROKE_WIDTH
                << "\""
                << " />\n";

            // Pin-purpose layers are crossed so they remain distinguishable
            // from routing geometry that uses the same fill and stroke color.
            if (shape.getLayer() == domain::Layer::M1_PIN || shape.getLayer() == domain::Layer::M2_PIN)
            {
                writePinCross(output, shape, transform);
            }

            output << "  </g>\n";
        }

        void writeViolationPoint(std::ofstream& output, double x, double y)
        {
            output
                << "    <circle "
                << "class=\"violation-point\" "
                << "cx=\"" << x << "\" "
                << "cy=\"" << y << "\" "
                << "r=\""
                << VIOLATION_POINT_RADIUS
                << "\" "
                << "fill=\"red\""
                << " />\n";
        }

        void writeViolationMarker(std::ofstream& output, const domain::Violation& violation, const SvgTransform& transform)
        {
            const auto& marker = violation.getMarker();

            if (!marker || !marker->firstPoint || !marker->secondPoint)
            {
                return;
            }

            const geometry::Point& firstPoint = *marker->firstPoint;
            const geometry::Point& secondPoint = *marker->secondPoint;

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

            writeViolationPoint(output, x1, y1);
            writeViolationPoint(output, x2, y2);

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

            if (marker->firstLayer)
            {
                output
                    << " "
                    << getLayerStyle(*marker->firstLayer).name;

                if (marker->secondLayer)
                {
                    output
                        << " With "
                        << getLayerStyle(*marker->secondLayer).name;
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

        void writeViolationRegion(std::ofstream& output, const domain::Violation& violation, const SvgTransform& transform)
        {
            const auto& marker = violation.getMarker();

            if (!marker || !marker->region)
            {
                return;
            }

            const auto& region = *marker->region;
            const double x = transform.toSvgX(region.getMinX());
            const double y = transform.toSvgY(region.getMaxY());
            const double width = transform.scaleLength(region.getMaxX() - region.getMinX());
            const double height = transform.scaleLength(region.getMaxY() - region.getMinY());

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

        void writeLegendItem(std::ofstream& output, domain::Layer layer, double x, double y)
        {
            const LayerStyle layerStyle = getLayerStyle(layer);
            const std::string className = std::string("layer-") + layerStyle.name;

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
                << "fill=\"" << layerStyle.fillColor << "\" "
                << "fill-opacity=\"0.35\" "
                << "stroke=\"" << layerStyle.strokeColor << "\" "
                << "stroke-width=\"2\""
                << " />\n";

            output
                << "      <text "
                << "class=\"legend-label\" "
                << "x=\"" << x + LEGEND_COLOR_SIZE + 10.0 << "\" "
                << "y=\"" << y + LEGEND_COLOR_SIZE - 3.0 << "\" "
                << "font-size=\"" << LEGEND_FONT_SIZE << "\" "
                << "fill=\"black\">"
                << layerStyle.name
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

        void writeLegend(std::ofstream& output, const std::vector<domain::Layer>& layers)
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
                << "Layers"
                << "</text>\n"
                << "    <text "
                << "id=\"toggle-all-layers\" "
                << "x=\"" << x + 65.0 << "\" "
                << "y=\"" << y << "\" "
                << "font-size=\"15\" "
                << "fill=\"#555555\" "
                << "text-decoration=\"underline\" "
                << "style=\"cursor: pointer;\" "
                << "onclick=\"toggleAllLayers()\">"
                << "Hide All"
                << "</text>\n";

            double itemY = y + 25.0;

            for (const domain::Layer layer : layers)
            {
                writeLegendItem(output, layer, x, itemY);
                itemY += LEGEND_ITEM_HEIGHT;
            }

            output << "  </g>\n";
        }

        std::string buildViolationSummary(const domain::Violation& violation)
        {
            std::ostringstream summary;

            summary << violation.getTypeAsString();

            if (const auto& marker = violation.getMarker(); marker && marker->firstLayer)
            {
                summary << " " << getLayerStyle(*marker->firstLayer).name;

                if (marker->secondLayer)
                {
                    summary << "-" << getLayerStyle(*marker->secondLayer).name;
                }
            }
            
            summary
                << " "
                << roundForReport(violation.getActualValue())
                << "/"
                << roundForReport(violation.getRequiredValue());

            return summary.str();
        }

        void writePanelButton(std::ofstream& output, double x, double y, const char* label, const char* clickHandler)
        {
            output
                << "    <g class=\"panel-button\" onclick=\"" << clickHandler << "\" style=\"cursor: pointer;\">\n"
                << "      <rect "
                << "x=\"" << x << "\" "
                << "y=\"" << y << "\" "
                << "width=\"" << PANEL_BUTTON_WIDTH << "\" "
                << "height=\"" << PANEL_BUTTON_HEIGHT << "\" "
                << "fill=\"#eeeeee\" "
                << "stroke=\"#999999\" "
                << "stroke-width=\"1\""
                << " />\n"
                << "      <text "
                << "x=\"" << x + PANEL_BUTTON_WIDTH / 2.0 << "\" "
                << "y=\"" << y + 16.0 << "\" "
                << "font-size=\"12\" "
                << "text-anchor=\"middle\" "
                << "fill=\"black\">"
                << label
                << "</text>\n"
                << "    </g>\n";
        }

        void writeViolationList(std::ofstream& output, const std::vector<domain::Violation>& violations, const std::vector<std::string>& violationLayerClasses, std::size_t totalPages)
        {
            const double x = SVG_PADDING;
            const double y = SVG_PADDING;
            const double width = VIOLATION_PANEL_WIDTH - 20.0;

            output << "  <g id=\"violation-list\">\n";

            output
                << "    <rect "
                << "x=\"" << x << "\" "
                << "y=\"" << y << "\" "
                << "width=\"" << width << "\" "
                << "height=\"" << SVG_HEIGHT - 2.0 * SVG_PADDING << "\" "
                << "fill=\"#fafafa\" "
                << "stroke=\"#cccccc\" "
                << "stroke-width=\"1\""
                << " />\n";

            output
                << "    <text "
                << "x=\"" << x + 10.0 << "\" "
                << "y=\"" << y + 20.0 << "\" "
                << "font-size=\"" << PANEL_TITLE_FONT_SIZE << "\" "
                << "font-weight=\"bold\" "
                << "fill=\"black\">"
                << "Violations: "
                << violations.size()
                << "</text>\n";

            const double buttonY = y + 30.0;
            writePanelButton(output, x + 10.0, buttonY, "Show All", "showAllViolations()");
            writePanelButton(output, x + 100.0, buttonY, "Hide All", "hideAllViolations()");

            for (std::size_t i = 0; i < violations.size(); ++i)
            {
                const auto& violation = violations[i];
                const std::size_t page = i / VIOLATIONS_PER_PAGE;
                const std::size_t indexInPage = i % VIOLATIONS_PER_PAGE;
                const double itemY = y + 70.0 + indexInPage * (VIOLATION_ITEM_HEIGHT + 4.0);

                output
                    << "    <g "
                    << "id=\"violation-item-" << i << "\" "
                    << "class=\"violation-list-item\" "
                    << "data-violation-id=\"violation-" << i << "\" "
                    << "data-message=\"" << violation.getMessage() << "\" "
                    << "data-page=\"" << page << "\" "
                    << "data-layers=\""
                    << violationLayerClasses[i]
                    << "\" "
                    << "onclick=\"toggleViolation(this)\" "
                    << "style=\"cursor: pointer;";

                if (page != 0)
                {
                    output << " display: none;";
                }

                output << "\">\n";

                output
                    << "      <rect "
                    << "x=\"" << x + 10.0 << "\" "
                    << "y=\"" << itemY - 14.0 << "\" "
                    << "width=\"" << width - 20.0 << "\" "
                    << "height=\"" << VIOLATION_ITEM_HEIGHT << "\" "
                    << "fill=\"white\" "
                    << "stroke=\"#dddddd\" "
                    << "stroke-width=\"1\""
                    << " />\n";

                output
                    << "      <text "
                    << "x=\"" << x + 18.0 << "\" "
                    << "y=\"" << itemY + 4.0 << "\" "
                    << "font-size=\"" << PANEL_TEXT_FONT_SIZE << "\" "
                    << "fill=\"black\">"
                    << (i + 1)
                    << ". "
                    << buildViolationSummary(violation)
                    << "</text>\n";

                output << "    </g>\n";
            }

            const double navigationY = SVG_HEIGHT - SVG_PADDING - 20.0;

            output
                << "    <text "
                << "x=\"" << x + 20.0 << "\" "
                << "y=\"" << navigationY << "\" "
                << "font-size=\"13\" "
                << "fill=\"#555555\" "
                << "style=\"cursor: pointer;\" "
                << "onclick=\"previousViolationPage()\">"
                << "Prev"
                << "</text>\n"
                << "    <text "
                << "id=\"violation-page-indicator\" "
                << "x=\"" << x + width / 2.0 << "\" "
                << "y=\"" << navigationY << "\" "
                << "font-size=\"13\" "
                << "text-anchor=\"middle\" "
                << "fill=\"black\">"
                << "1 / " << totalPages
                << "</text>\n"
                << "    <text "
                << "x=\"" << x + width - 45.0 << "\" "
                << "y=\"" << navigationY << "\" "
                << "font-size=\"13\" "
                << "fill=\"#555555\" "
                << "style=\"cursor: pointer;\" "
                << "onclick=\"nextViolationPage()\">"
                << "Next"
                << "</text>\n";

            const double detailsX = x + 10.0;
            const double detailsY = SVG_HEIGHT - SVG_PADDING - 130.0;
            const double detailsWidth = width - 20.0;
            const double detailsHeight = 80.0;

            output
                << "    <rect "
                << "x=\"" << detailsX << "\" "
                << "y=\"" << detailsY << "\" "
                << "width=\"" << detailsWidth << "\" "
                << "height=\"" << detailsHeight << "\" "
                << "fill=\"#fafafa\" "
                << "stroke=\"#cccccc\" "
                << "stroke-width=\"1\" "
                << "rx=\"4\""
                << " />\n";

            output
                << "    <text "
                << "x=\"" << detailsX + 8.0 << "\" "
                << "y=\"" << detailsY + 18.0 << "\" "
                << "font-size=\"13\" "
                << "font-weight=\"bold\" "
                << "fill=\"black\">"
                << "Violation Details"
                << "</text>\n";

            output
                << "    <text "
                << "id=\"violation-details-text\" "
                << "x=\"" << detailsX + 8.0 << "\" "
                << "y=\"" << detailsY + 40.0 << "\" "
                << "font-size=\"13\" "
                << "fill=\"#555555\">"
                << "Select a violation"
                << "</text>\n";

            output << "  </g>\n";
        }

        void writeSvgOpening(std::ofstream& output)
        {
            output
                << "<svg "
                << "xmlns=\"http://www.w3.org/2000/svg\" "
                << "width=\"100%\" "
                << "height=\"90vh\" "
                << "viewBox=\"0 0 "
                << SVG_WIDTH << " "
                << SVG_HEIGHT << "\" "
                << "preserveAspectRatio=\"xMidYMid meet\">\n";
        }

        void writeStyles(std::ofstream& output)
        {
            output << R"SVG(  <style>
    .violation-label {
      visibility: hidden;
      pointer-events: none;
    }
    .violation-overlay:hover .violation-label {
      visibility: visible;
    }
    .violation-overlay {
      cursor: pointer;
    }
    .legend-item.disabled .legend-label {
      fill: darkgray;
    }
    .legend-item.disabled rect {
      opacity: 0.35;
    }
    .violation-list-item.active rect {
      fill: #ffeaea;
      stroke: #cc0000;
    }
    .violation-list-item.active text {
      fill: #990000;
    }
  </style>
)SVG";
        }

        void writeScript(std::ofstream& output, std::size_t totalViolationPages)
        {
            // A raw string keeps the generated JavaScript readable and avoids
            // maintaining an escape-heavy stream expression line by line.
            output << R"JS(  <script><![CDATA[
    let currentViolationPage = 0;
    const totalViolationPages = )JS" << totalViolationPages << R"JS(;

function setWrappedSvgText(textElement, message, maxWidth, lineHeight, maxLines) {
    while (textElement.firstChild) {
        textElement.removeChild(textElement.firstChild);
    }

    const x = textElement.getAttribute('x');
    const words = message.split(/\s+/);

    let line = '';
    let lineCount = 0;
    let tspan = document.createElementNS('http://www.w3.org/2000/svg', 'tspan');

    tspan.setAttribute('x', x);
    tspan.setAttribute('dy', '0');
    textElement.appendChild(tspan);

    for (let i = 0; i < words.length; ++i) {
        const testLine = line === '' ? words[i] : line + ' ' + words[i];
        tspan.textContent = testLine;

        if (tspan.getComputedTextLength() > maxWidth && line !== '') {
            tspan.textContent = line;
            lineCount++;

            if (lineCount >= maxLines) {
                let truncated = line;

                while (truncated.length > 0) {
                    tspan.textContent = truncated + '...';

                    if (tspan.getComputedTextLength() <= maxWidth) {
                        break;
                    }

                    truncated = truncated.slice(0, -1);
                }

                return;
            }

            line = words[i];

            tspan = document.createElementNS('http://www.w3.org/2000/svg', 'tspan');
            tspan.setAttribute('x', x);
            tspan.setAttribute('dy', lineHeight);
            tspan.textContent = line;
            textElement.appendChild(tspan);
        }
        else {
            line = testLine;
        }
    }

    tspan.textContent = line;
}

function setViolationDetails(message) {
    const details = document.getElementById('violation-details-text');

    if (!details) {
        return;
    }

    const text = message && message.trim() !== ''
        ? message
        : 'Select a violation';

    setWrappedSvgText(details, text, 260, 14, 4);
}

function toggleViolation(item) {
    const violationId = item.dataset.violationId;
    const overlay = document.getElementById(violationId);

    if (!overlay) {
        return;
    }

    const isActive = item.classList.toggle('active');

    overlay.style.display = isActive ? '' : 'none';

    if (isActive) {
        setViolationDetails(item.dataset.message);
    }
    else {
        const activeItems =
            document.querySelectorAll('.violation-list-item.active');

        if (activeItems.length === 0) {
            setViolationDetails('Select a violation');
        }
        else {
            setViolationDetails('Multiple violations selected');
        }
    }
}

function showAllViolations() {
    document.querySelectorAll('.violation-list-item').forEach(function(item) {
        item.classList.add('active');

        const overlay = document.getElementById(item.dataset.violationId);

        if (overlay && item.style.display !== 'none') {
            overlay.style.display = '';
        }
    });

    setViolationDetails('All violations are shown');
}

function hideAllViolations() {
    document.querySelectorAll('.violation-list-item').forEach(function(item) {
        item.classList.remove('active');

        const overlay = document.getElementById(item.dataset.violationId);

        if (overlay) {
            overlay.style.display = 'none';
        }
    });

    setViolationDetails('Select a violation');
}
    function getHiddenLayers() {
      const hiddenLayers = new Set();

      document.querySelectorAll('.legend-item.disabled').forEach(function(item) {
        hiddenLayers.add(item.dataset.layer);
      });

      return hiddenLayers;
    }

    function itemUsesHiddenLayer(item, hiddenLayers) {
      const layersText = item.dataset.layers || '';
      const layers = layersText === '' ? [] : layersText.split(' ');

      return layers.some(function(layer) {
        return hiddenLayers.has(layer);
      });
    }

    function updateViolationListVisibility(hiddenLayers) {
      document.querySelectorAll('.violation-list-item').forEach(function(item) {
        const hiddenByLayer = itemUsesHiddenLayer(item, hiddenLayers);
        const onCurrentPage = Number(item.dataset.page) === currentViolationPage;

        item.style.display = !hiddenByLayer && onCurrentPage ? '' : 'none';

        if (hiddenByLayer) {
          item.classList.remove('active');
        }

        // Pagination only changes the list; selected overlays stay visible
        // across pages unless their layer is hidden.
        const overlay = document.getElementById(item.dataset.violationId);
        if (overlay) {
          const showOverlay = !hiddenByLayer && item.classList.contains('active');
          overlay.style.display = showOverlay ? '' : 'none';
        }
      });
    }

    function updateLayerVisibility() {
      const hiddenLayers = getHiddenLayers();

      document.querySelectorAll('.shape').forEach(function(shape) {
        let hideShape = false;

        shape.classList.forEach(function(className) {
          if (className.startsWith('layer-') && hiddenLayers.has(className)) {
            hideShape = true;
          }
        });

        shape.style.display = hideShape ? 'none' : '';
      });

      updateViolationListVisibility(hiddenLayers);
    }

    function updateViolationPage() {
      updateViolationListVisibility(getHiddenLayers());

      const indicator = document.getElementById('violation-page-indicator');
      if (indicator) {
        indicator.textContent =
          (currentViolationPage + 1) + ' / ' + totalViolationPages;
      }
    }

    function nextViolationPage() {
      if (currentViolationPage + 1 < totalViolationPages) {
        currentViolationPage++;
        updateViolationPage();
      }
    }

    function previousViolationPage() {
      if (currentViolationPage > 0) {
        currentViolationPage--;
        updateViolationPage();
      }
    }

    function updateToggleAllText() {
      const legendItems = document.querySelectorAll('.legend-item');
      const toggleText = document.getElementById('toggle-all-layers');

      if (!toggleText || legendItems.length === 0) {
        return;
      }

      const allHidden = Array.from(legendItems).every(function(item) {
        return item.classList.contains('disabled');
      });

      toggleText.textContent = allHidden ? 'Show All' : 'Hide All';
    }

    function toggleAllLayers() {
      const legendItems = document.querySelectorAll('.legend-item');

      const allHidden = Array.from(legendItems).every(function(item) {
        return item.classList.contains('disabled');
      });

      legendItems.forEach(function(item) {
        if (allHidden) {
          item.classList.remove('disabled');
        }
        else {
          item.classList.add('disabled');
        }
      });

      updateLayerVisibility();
      updateToggleAllText();
    }

    function toggleLayer(legendItem) {
      legendItem.classList.toggle('disabled');
      updateLayerVisibility();
      updateToggleAllText();
    }

    const svg = document.documentElement;

    const originalWidth = )JS" << SVG_WIDTH << R"JS(;
    const originalHeight = )JS" << SVG_HEIGHT << R"JS(;

    let viewX = 0;
    let viewY = 0;
    let viewWidth = originalWidth;
    let viewHeight = originalHeight;

    const MIN_ZOOM_FACTOR = 0.1;

    svg.addEventListener('wheel', function(event) {
      event.preventDefault();

      const rect = svg.getBoundingClientRect();

      const mouseX = viewX +
        ((event.clientX - rect.left) / rect.width) * viewWidth;

      const mouseY = viewY +
        ((event.clientY - rect.top) / rect.height) * viewHeight;

      let zoomFactor = event.deltaY < 0 ? 0.9 : 1.1;
      let newWidth = viewWidth * zoomFactor;
      const minWidth = originalWidth * MIN_ZOOM_FACTOR;

      if (newWidth < minWidth) {
        newWidth = minWidth;
      }

      if (newWidth > originalWidth) {
        newWidth = originalWidth;
      }

      zoomFactor = newWidth / viewWidth;

      const newHeight = viewHeight * zoomFactor;

      viewX = mouseX - (mouseX - viewX) * zoomFactor;
      viewY = mouseY - (mouseY - viewY) * zoomFactor;

      viewWidth = newWidth;
      viewHeight = newHeight;

      if (viewWidth == originalWidth) {
        viewX = 0;
        viewY = 0;
        viewHeight = originalHeight;
      }

      svg.setAttribute(
        'viewBox',
        viewX + ' ' + viewY + ' ' + viewWidth + ' ' + viewHeight
      );
    }, { passive: false });

    function resetView() {
      viewX = 0;
      viewY = 0;
      viewWidth = originalWidth;
      viewHeight = originalHeight;

      svg.setAttribute(
        'viewBox',
        '0 0 ' + originalWidth + ' ' + originalHeight
      );
    }

    document.addEventListener('keydown', function(event) {
      if (event.key === 'f' || event.key === 'F') {
        resetView();
      }
    });
  ]]></script>
)JS";
        }

    } // namespace

    void SVGReportWriter::write(const std::vector<domain::Shape>& shapes, const std::vector<domain::Violation>& violations, const std::string& filePath)
    {
        std::ofstream output(filePath);

        if (!output.is_open())
        {
            throw std::runtime_error("Failed to open SVG output file");
        }

        const auto reportBounds = calculateReportBounds(shapes, violations);

        writeSvgOpening(output);

        if (!reportBounds)
        {
            output << "</svg>\n";
            return;
        }

        const SvgTransform transform(*reportBounds);
        writeStyles(output);

        std::vector<std::string> violationLayerClasses;
        violationLayerClasses.reserve(violations.size());

        for (const auto& violation : violations)
        {
            violationLayerClasses.push_back(getViolationLayerClasses(violation, shapes));
        }

        const std::size_t violationPageCount = getViolationPageCount(violations.size());

        output << "  <g id=\"layout\">\n";

        const auto orderedShapes = getShapesInDrawOrder(shapes);

        for (const auto& shape : orderedShapes)
        {
            writePolygon(output, shape, transform);
        }

        output << "  </g>\n";

        output << "  <g id=\"violations\">\n";

        for (std::size_t i = 0; i < violations.size(); ++i)
        {
            const auto& violation = violations[i];

            output
                << "    <g "
                << "id=\"violation-" << i << "\" "
                << "class=\"violation-overlay\" "
                << "data-layers=\""
                << violationLayerClasses[i]
                << "\" "
                << "style=\"display: none;\">\n";

            writeViolationEdges(output, violation, shapes, transform);
            writeViolationMarker(output, violation, transform);
            writeViolationRegion(output, violation, transform);

            output << "    </g>\n";
        }

        output << "  </g>\n";

        writeViolationList(output, violations, violationLayerClasses, violationPageCount);

        const auto existingLayers = getExistingLayers(shapes);
        writeLegend(output, existingLayers);

        writeScript(output, violationPageCount);

        output << "</svg>\n";
    }

}

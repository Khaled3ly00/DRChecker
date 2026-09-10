#include "drcheck/gui/LayoutViewModel.h"

#include <QVariantList>

namespace drcheck::gui {
LayoutViewModel::LayoutViewModel(QObject* parent)
    : QObject{ parent }
{
}

bool LayoutViewModel::hasLayout() const
{
    return !layerPolygons.empty();
}

double LayoutViewModel::getMinX() const
{
    return minX;
}

double LayoutViewModel::getMinY() const
{
    return minY;
}

double LayoutViewModel::getMaxX() const
{
    return maxX;
}

double LayoutViewModel::getMaxY() const
{
    return maxY;
}

QVariantMap LayoutViewModel::getViolationMarker() const
{
    return violationMarker;
}

const std::optional<domain::ViolationMarker>& LayoutViewModel::getSelectedViolationMarker() const
{
    return selectedViolationMarker;
}

const LayoutViewModel::LayerPolygonMap& LayoutViewModel::getLayerPolygons() const
{
    return layerPolygons;
}

const std::vector<qulonglong>& LayoutViewModel::getSelectedViolationShapeIds() const
{
    return selectedViolationShapeIds;
}

bool LayoutViewModel::hasViolationMarker() const
{
    return !violationMarker.isEmpty();
}

std::optional<geometry::BoundingBox> LayoutViewModel::calculateLayoutBounds(const std::vector<domain::Shape>& shapes) const
{
    std::optional<geometry::BoundingBox> bounds;

    for (const auto& shape : shapes)
    {
        const auto shapeBounds = shape.getPolygon().getBoundingBox();

        bounds = bounds ? bounds->mergedWith(shapeBounds) : shapeBounds;
    }

    return bounds;
}

void LayoutViewModel::setShapes(const std::vector<domain::Shape>& shapes)
{
    layerPolygons.clear();

    for (const auto& shape : shapes)
    {

        const QString layerName = QString::fromStdString(shape.getLayer()->getName());

        const auto& polygonVertices = shape.getPolygon().getVertices();

        std::vector<QPointF> renderPolygon;

        renderPolygon.reserve(polygonVertices.size());

        for (std::size_t i = 0; i < polygonVertices.size(); ++i)
        {
            const auto& vertex = polygonVertices[i];

            const auto& nextVertex = polygonVertices[ (i + 1) % polygonVertices.size()];

            renderPolygon.emplace_back(vertex.getX(), vertex.getY());
        }

        layerPolygons[layerName].emplace_back(static_cast<qulonglong>(shape.getId()), std::move(renderPolygon));
    }

    const auto bounds = calculateLayoutBounds(shapes);

    if (bounds.has_value())
    {
        minX = bounds->getMinX();
        minY = bounds->getMinY();
        maxX = bounds->getMaxX();
        maxY = bounds->getMaxY();
    }
    else
    {
        minX = 0.0;
        minY = 0.0;
        maxX = 0.0;
        maxY = 0.0;
    }

    emit layoutChanged();
}

void LayoutViewModel::setSelectedViolation(const domain::Violation& violation)
{
    selectedViolationShapeIds.clear();
    selectedViolationMarker.reset();

    for (const std::size_t shapeId : violation.getShapeIds())
    {
        selectedViolationShapeIds.push_back(static_cast<qulonglong>(shapeId));
    }

    const auto& marker = violation.getMarker();

    if (!marker.has_value())
    {
        clearSelectedViolation();
        return;
    }

    selectedViolationMarker = marker;

    QVariantMap markerData;

    if (marker->firstPoint.has_value())
    {
        markerData["hasFirstPoint"] = true;
        markerData["firstX"] = marker->firstPoint->getX();
        markerData["firstY"] = marker->firstPoint->getY();
    }

    if (marker->secondPoint.has_value())
    {
        markerData["hasSecondPoint"] = true;
        markerData["secondX"] = marker->secondPoint->getX();
        markerData["secondY"] = marker->secondPoint->getY();
    }

    if (marker->region.has_value())
    {
        markerData["hasRegion"] = true;
        markerData["regionMinX"] = marker->region->getMinX();
        markerData["regionMinY"] = marker->region->getMinY();
        markerData["regionMaxX"] = marker->region->getMaxX();
        markerData["regionMaxY"] = marker->region->getMaxY();
    }

    violationMarker = std::move(markerData);

    emit violationMarkerChanged();

}

void LayoutViewModel::clearSelectedViolation()
{
    if (violationMarker.isEmpty() && selectedViolationShapeIds.empty()) {
        return;
    }

    violationMarker.clear();
    selectedViolationShapeIds.clear();
    selectedViolationMarker.reset();
    emit violationMarkerChanged();
}

void LayoutViewModel::clear()
{
    if (layerPolygons.empty()) {
        return;
    }

    layerPolygons.clear();

    minX = 0.0;
    minY = 0.0;
    maxX = 0.0;
    maxY = 0.0;

    emit layoutChanged();
}
}
#include "drcheck/gui/LayoutViewModel.h"

#include <QVariantList>

namespace drcheck::gui {
LayoutViewModel::LayoutViewModel(QObject* parent)
    : QObject{ parent }
{
}

const QVariantList& LayoutViewModel::getPolygons() const
{
    return polygons;
}

bool LayoutViewModel::hasLayout() const
{
    return !polygons.empty();
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
    polygons.clear();

    for (const auto& shape : shapes)
    {
        QVariantList vertices;

        for (const auto& vertex : shape.getPolygon().getVertices())
        {
            QVariantMap pointData;

            pointData["x"] = vertex.getX();
            pointData["y"] = vertex.getY();

            vertices.append(pointData);
        }

        QVariantMap polygonData;

        polygonData["layerName"] = QString::fromStdString(shape.getLayer()->getName());

        polygonData["vertices"] = vertices;

        polygons.append(polygonData);
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
    const auto& marker = violation.getMarker();

    if (!marker.has_value())
    {
        clearSelectedViolation();
        return;
    }

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
    if (violationMarker.isEmpty()) {
        return;
    }

    violationMarker.clear();

    emit violationMarkerChanged();
}

void LayoutViewModel::clear()
{
    if (polygons.empty()) {
        return;
    }

    polygons.clear();

    minX = 0.0;
    minY = 0.0;
    maxX = 0.0;
    maxY = 0.0;

    emit layoutChanged();
}
}
#pragma once

#include <QObject>
#include <QVariantMap>
#include <optional>
#include <vector>
#include <QHash>
#include <QPointF>
#include <utility>

#include "drcheck/domain/Shape.h"
#include "drcheck/geometry/BoundingBox.h"
#include "drcheck/domain/Violation.h"

namespace drcheck::gui {

class LayoutViewModel : public QObject
{

    Q_OBJECT

    Q_PROPERTY(bool hasLayout READ hasLayout NOTIFY layoutChanged)
    Q_PROPERTY(double minX READ getMinX NOTIFY layoutChanged)
    Q_PROPERTY(double minY READ getMinY NOTIFY layoutChanged)
    Q_PROPERTY(double maxX READ getMaxX NOTIFY layoutChanged)
    Q_PROPERTY(double maxY READ getMaxY NOTIFY layoutChanged)
    Q_PROPERTY(QVariantMap violationMarker READ getViolationMarker NOTIFY violationMarkerChanged)
    Q_PROPERTY(bool hasViolationMarker READ hasViolationMarker NOTIFY violationMarkerChanged)

    // unordered map between layers and vector of polygons vertices&IDs
    using LayerPolygon = std::pair<qulonglong, std::vector<QPointF>>;
    using LayerPolygonMap = QHash<QString, std::vector<LayerPolygon>>;

public:
    explicit LayoutViewModel(QObject* parent = nullptr);

    bool hasLayout() const;

    double getMinX() const;
    double getMinY() const;
    double getMaxX() const;
    double getMaxY() const;
    QVariantMap getViolationMarker() const;
    const LayerPolygonMap& getLayerPolygons() const;
    const std::vector<qulonglong>& getSelectedViolationShapeIds() const;
    const std::optional<domain::ViolationMarker>& getSelectedViolationMarker() const;

    void setShapes(const std::vector<domain::Shape>& shapes);

    bool hasViolationMarker() const;

    void setSelectedViolation(const domain::Violation& violation);

    void clearSelectedViolation();

    void clear();

signals:
    void layoutChanged();
    void violationMarkerChanged();

private:
    QVariantMap violationMarker;

    LayerPolygonMap layerPolygons;

    std::vector<qulonglong> selectedViolationShapeIds;

    std::optional<geometry::BoundingBox> calculateLayoutBounds(const std::vector<domain::Shape>& shapes) const;
    std::optional<domain::ViolationMarker> selectedViolationMarker;

    QString generateLayerColor(std::size_t index) const;

    double minX = 0.0;
    double minY = 0.0;
    double maxX = 0.0;
    double maxY = 0.0;
};
}
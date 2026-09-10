#pragma once

#include <QQuickItem>
#include <QStringList>

#include "drcheck/gui/LayoutViewModel.h"
#include "drcheck/gui/LayerListModel.h"

namespace drcheck::gui {

class LayoutRenderItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(double viewScale READ getViewScale WRITE setViewScale NOTIFY viewScaleChanged)
    Q_PROPERTY(QObject* layoutModel READ getLayoutModel WRITE setLayoutModel NOTIFY layoutModelChanged)
    Q_PROPERTY(QObject* layersModel READ getLayersModel WRITE setLayersModel NOTIFY layersModelChanged)

public:
    explicit LayoutRenderItem(QQuickItem* parent = nullptr);

    QObject* getLayoutModel() const;
    QObject* getLayersModel() const;
    double getViewScale() const;

    void setViewScale(double scale);
    void setLayersModel(QObject* model);
    void setLayoutModel(QObject* model);

signals:
    void layoutModelChanged();
    void layersModelChanged();
    void viewScaleChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;

private:
    double viewScale = 1.0;

    LayoutViewModel* layoutModel = nullptr;
    LayerListModel* layersModel = nullptr;

    QStringList layerOrder;

    bool geometryDirty = true;
    bool stylesDirty = true;
    bool selectedLayerDirty = true;
    bool violationDirty = true;
    bool markerDirty = true;

    void handleLayoutChanged();
    void handleLayerStylesChanged();
    void handleViolationSelectionChanged();
    void handleHighlightedLayerChanged();

    void rebuildNormalLayers(QSGNode* parentNode);
    void rebuildSelectedLayer(QSGNode* parentNode);
    void rebuildViolationShapes(QSGNode* parentNode);
    void rebuildViolationMarker(QSGNode* parentNode);
    void updateLayerStyles(QSGNode* normalLayersNode, QSGNode* selectedLayerNode);

    void clearChildren(QSGNode* node);

    // Triangulation for fill
    static double cross(const QPointF& first, const QPointF& second, const QPointF& third);
    static bool pointInTriangle(const QPointF& point, const QPointF& first, const QPointF& second, const QPointF& third);
    static std::vector<QPointF> triangulatePolygon(const std::vector<QPointF>& polygon);

    static void appendThickLine(std::vector<QPointF>& triangles, const QPointF& first, const QPointF& second, double thickness);
};

}
#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QVariantMap>
#include <QColor>

#include "drcheck/domain/Shape.h"

namespace drcheck::gui {
class LayerListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ getCount NOTIFY countChanged)

    Q_PROPERTY(QVariantMap layerStyles READ getLayerStyles NOTIFY layerStylesChanged)

    Q_PROPERTY(bool allVisible READ getAllVisible NOTIFY allVisibleChanged)

public:
    enum LayerRole
    {
        LayerNameRole = Qt::UserRole + 1,
        LayerColorRole,
        LayerVisibleRole
    };

    Q_ENUM(LayerRole)

    explicit LayerListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    int getCount() const;

    QVariantMap getLayerStyles() const;

    void setLayers(const std::vector<domain::Shape>& shapes);

    Q_INVOKABLE void setLayerVisible(int row, bool visible);

    bool getAllVisible() const;

    Q_INVOKABLE void setAllLayersVisible(bool visible);

    Q_INVOKABLE void setLayerColor(int row, const QColor& color);

    void clear();

signals:
    void countChanged();
    void layerStylesChanged();
    void allVisibleChanged();

private:
    struct LayerDisplayData
    {
        QString name;
        QString color;
        bool visible = true;
    };

    QString generateLayerColor(std::size_t index) const;

    std::vector<LayerDisplayData> layers;
};
}
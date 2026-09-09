#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <optional>

#include "drcheck/gui/ViolationListModel.h"
#include "drcheck/gui/LayoutViewModel.h"
#include "drcheck/gui/LayerListModel.h"
#include "drcheck/engine/DRCRunner.h"

namespace drcheck::gui {

class AppController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString layoutPath READ getLayoutPath NOTIFY layoutPathChanged)
    Q_PROPERTY(QString rulePath READ getRulePath NOTIFY rulePathChanged)
    Q_PROPERTY(QString jsonReportDirectory READ getJsonReportDirectory NOTIFY jsonReportDirectoryChanged)
    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)

public:
    explicit AppController(ViolationListModel& violationModel, LayoutViewModel& layoutViewModel, LayerListModel& layerModel, QObject* parent = nullptr);

    Q_INVOKABLE void setLayoutFile(const QUrl& fileUrl);
    Q_INVOKABLE void setRuleFile(const QUrl& fileUrl);
    Q_INVOKABLE void setJsonReportDirectory(const QUrl& fileUrl);
    Q_INVOKABLE void selectViolation(int row);
    Q_INVOKABLE void runDRC();

    QString getLayoutPath() const;
    QString getRulePath() const;
    QString getJsonReportDirectory() const;
    QString getStatus() const;

signals:
    void layoutPathChanged();
    void rulePathChanged();
    void jsonReportDirectoryChanged();
    void statusChanged();

private:
    std::optional<engine::DRCRunResult> currentResult;

    void setStatus(const QString& newStatus);

    QString layoutPath;
    QString rulePath;
    QString jsonReportDirectory;
    QString status = "Ready";

    int violationCount = 0;

    ViolationListModel& violationModel;
    LayoutViewModel& layoutViewModel;
    LayerListModel& layerModel;
};

}
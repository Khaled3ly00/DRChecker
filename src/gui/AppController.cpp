#include "drcheck/gui/AppController.h"

#include <QDir>

namespace drcheck::gui {

AppController::AppController(ViolationListModel& violationModel, LayoutViewModel& layoutViewModel, LayerListModel& layerModel, QObject* parent)
    : QObject{ parent }, violationModel{ violationModel }, layoutViewModel{ layoutViewModel }, layerModel{layerModel}
{
}

void AppController::setLayoutFile(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (layoutPath == path) {
        return;
    }

    layoutPath = path;
    emit layoutPathChanged();
}

QString AppController::getLayoutPath() const
{
    return layoutPath;
}

void AppController::setRuleFile(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (rulePath == path) {
        return;
    }

    rulePath = path;
    emit rulePathChanged();
}

QString AppController::getRulePath() const
{
    return rulePath;
}

void AppController::setJsonReportDirectory(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (jsonReportDirectory == path) {
        return;
    }

    jsonReportDirectory = path;
    emit jsonReportDirectoryChanged();
}

QString AppController::getJsonReportDirectory() const
{
    return jsonReportDirectory;
}

QString AppController::getStatus() const
{
    return status;
}

void AppController::setStatus(const QString& newStatus)
{
    if (status == newStatus) {
        return;
    }

    status = newStatus;
    emit statusChanged();
}

void AppController::selectViolation(int row)
{
    if (!currentResult.has_value() || row < 0 || static_cast<std::size_t>(row) >= currentResult->violations.size())
    {
        layoutViewModel.clearSelectedViolation();
        return;
    }

    layoutViewModel.setSelectedViolation(currentResult->violations[row]);
}

void AppController::runDRC()
{
    if (layoutPath.isEmpty() || rulePath.isEmpty() || jsonReportDirectory.isEmpty())
    {
        violationModel.clear();
        layoutViewModel.clear();
        layoutViewModel.clearSelectedViolation();
        layerModel.clear();
        currentResult.reset();
        setStatus("Layout, rule deck, and JSON report directory are required");
        return;
    }

    // Clear results from the previous run before starting a new DRC run.
    violationModel.clear();
    layoutViewModel.clear();
    layoutViewModel.clearSelectedViolation();
    layerModel.clear();
    currentResult.reset();
    setStatus("Running DRC...");

    try
    {
        engine::DRCRunConfig config;

        config.layoutPath = layoutPath.toStdString();
        config.rulesPath = rulePath.toStdString();

        const QString reportPath = QDir(jsonReportDirectory).filePath("report.json");

        config.reportPath = reportPath.toStdString();
        config.svgPath = std::nullopt;
        config.topCellName = std::nullopt;

        currentResult = engine::DRCRunner::run(config);

        layerModel.setLayers(currentResult->shapes);
        layoutViewModel.setShapes(currentResult->shapes);
        violationModel.setViolations(currentResult->violations);

        setStatus("DRC completed successfully");
    }
    catch (const std::exception& exception)
    {
        violationModel.clear();
        layoutViewModel.clear();
        layoutViewModel.clearSelectedViolation();
        layerModel.clear();
        currentResult.reset();

        setStatus("Error: " + QString::fromUtf8(exception.what()));
    }
}

}
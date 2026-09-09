#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "drcheck/gui/AppController.h"
#include "drcheck/gui/ViolationListModel.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    drcheck::gui::ViolationListModel violationModel;

    drcheck::gui::LayoutViewModel layoutViewModel;

    drcheck::gui::LayerListModel layerModel;

    drcheck::gui::AppController controller(violationModel, layoutViewModel, layerModel);

    // Expose to QML
    engine.rootContext()->setContextProperty("appController", &controller);

    engine.rootContext()->setContextProperty("violationModel", &violationModel);

    engine.rootContext()->setContextProperty("layoutViewModel", &layoutViewModel);

    engine.rootContext()->setContextProperty("layerModel", &layerModel);

    // URI: DRCheck
    engine.loadFromModule("DRCheck", "Main");

    return app.exec();
}
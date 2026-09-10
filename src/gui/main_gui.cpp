#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <qqml.h>

#include "drcheck/gui/AppController.h"
#include "drcheck/gui/ViolationListModel.h"
#include "drcheck/gui/LayoutRenderItem.h"

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

    qmlRegisterType<drcheck::gui::LayoutRenderItem>("DRCheck", 1, 0, "LayoutRenderItem");

    // URI: DRCheck
    engine.loadFromModule("DRCheck", "Main");

    return app.exec();
}
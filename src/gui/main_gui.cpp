#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "drcheck/gui/AppController.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    drcheck::gui::AppController controller;

    engine.rootContext()->setContextProperty("appController", &controller);

    // URI: DRCheck
    engine.loadFromModule("DRCheck", "Main");

    return app.exec();
}
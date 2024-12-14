#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>

#include "appdata.h"
#include "datacontroller.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QSharedPointer<AppData> appDataPtr = QSharedPointer<AppData>(new AppData(30, 10, 10));
    DataController datacontroller(appDataPtr);

    //  Create data
    if (!datacontroller.createDB(1000)) {
        qDebug() << "Cannot create database";
        return -1;
    }

    // Create and start thread
    QThread dataProviderThread;
    datacontroller.moveToThread(&dataProviderThread);

    QObject::connect(&dataProviderThread, &QThread::started, &datacontroller, &DataController::initCaching);
    dataProviderThread.start();


    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/Chat/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    QQmlContext* qmlContext = engine.rootContext();
    qmlContext->setContextProperty("appData", appDataPtr.data());


    // ???
    appDataPtr->requestLatestMessages(0);
    engine.load(url);

    // Run QML application
    int res = app.exec();

    // Stop thread
    dataProviderThread.quit();
    dataProviderThread.wait();

    return res;
}

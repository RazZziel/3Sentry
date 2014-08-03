#include <QApplication>

#include "controller.h"
#include "trackface/trackface.h"
#include "sentryui.h"
#include "webui.h"

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("i+D3");
    QCoreApplication::setOrganizationDomain("imasdetres.com");
    QCoreApplication::setApplicationName("3Sentry");
    QCoreApplication::setApplicationVersion("0.0.1");

#if 0
    return trackface(argc, argv);
#else
    Controller controller;

    QApplication a(argc, argv);
    SentryUI w(&controller);
    WebUI h(&controller);
    w.show();
    return a.exec();
#endif
}

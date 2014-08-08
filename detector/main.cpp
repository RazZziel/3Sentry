#include <QApplication>

#include "controller.h"
#include "sentryui.h"
#include "sentryweb.h"
#include "sentryinput.h"

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("i+D3");
    QCoreApplication::setOrganizationDomain("imasdetres.com");
    QCoreApplication::setApplicationName("3Sentry");
    QCoreApplication::setApplicationVersion("0.0.1");

    QApplication a(argc, argv);


    Controller controller;

    SentryWeb web(&controller);
    Q_UNUSED(web);

    SentryInput input(&controller);
    Q_UNUSED(input);

    controller.startProcessing();

#if 1
    SentryUI ui(&controller);
    ui.show();
#endif


    bool ret = a.exec();

    //controller.stopProcessing();

    return ret;
}

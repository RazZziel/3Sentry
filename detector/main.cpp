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
    input.start();
    controller.setInput(&input);

#if 1
    SentryUI ui(&controller);
    ui.show();
#endif


    bool ret = a.exec();

    for (int i=Hardware::EyeLaser; i<=Hardware::LeftGun; i++)
    {
        Hardware::Trigger trigger = (Hardware::Trigger) i;
        controller.hardware()->stopFiring(trigger);
    }

    return ret;
}

#include <QApplication>
#include <QMessageLogContext>
#include <QTextStream>

#include "controller.h"
#include "sentryui.h"
#include "sentryweb.h"
#include "sentryinput.h"

void debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    switch (type)
    {
        case QtDebugMsg:
            QTextStream(stdout) << "[" << context.category << "] " << msg << endl;
            break;
        case QtWarningMsg:
        case QtCriticalMsg:
        case QtFatalMsg:
            QTextStream(stderr) << "[" << context.category << "] " << msg << endl;
            break;
    }
}

int main(int argc, char** argv)
{
    qInstallMessageHandler(debugMessageHandler);

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

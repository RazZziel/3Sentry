#ifndef SENTRYINPUT_H
#define SENTRYINPUT_H

#include <QObject>
#include <QThread>

#include <QMap>

class Controller;

class SentryInput : public QThread
{
    Q_OBJECT
public:
    explicit SentryInput(Controller *controller);
    void run();

    void init();

private:
    Controller *m_controller;

    int m_max_joy;

};

#endif // SENTRYINPUT_H

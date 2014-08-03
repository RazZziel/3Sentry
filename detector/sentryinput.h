#ifndef SENTRYINPUT_H
#define SENTRYINPUT_H

#include <QObject>

class Controller;

class SentryInput : public QObject
{
    Q_OBJECT
public:
    explicit SentryInput(Controller *controller);

private:
    Controller *m_controller;
};

#endif // SENTRYINPUT_H

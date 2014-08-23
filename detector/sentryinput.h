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
    qreal m_axis_0;
    qreal m_axis_1;
    qreal m_axis_2;
    qreal m_axis_3;

    char m_left_fire_button;
    char m_right_fire_button;
    char m_laser_button;

};

#endif // SENTRYINPUT_H

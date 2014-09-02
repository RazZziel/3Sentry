#ifndef SENTRYINPUT_H
#define SENTRYINPUT_H

#include <QObject>
#include <QThread>
#include <QLoggingCategory>
#include <QHash>
#include <QDateTime>
#include <SDL_joystick.h>

#include "parametermanager.h"

class Controller;

class SentryInput : public QThread, public ParameterOwner
{
    Q_OBJECT
public:
    explicit SentryInput(Controller *controller);
    void run();

    void init();

    ParameterManager *parameterManager();
    QString settingsGroup();

protected:
    ParameterList createParameters() const;

private slots:
    void onParametersChanged();

private:
    struct Joystick {
        Joystick(SDL_Joystick *joystick);

        int index;

        int m_max_joy;

        char m_button_left_fire;
        char m_button_right_fire;
        char m_button_eyeLaser;
        char m_button_left_laser;
        char m_button_right_laser;

        char m_axis_body_x;
        char m_axis_body_y;
        char m_axis_laser_x;
        char m_axis_laser_y;

        QHash<int,QDateTime> m_doubleClickTimers;
    };

    Controller *m_controller;
    ParameterManager *m_parameterManager;
    QLoggingCategory cat;

    QHash<SDL_JoystickID, Joystick*> m_joysticks;

    int m_dead_zone_radius;
};

#endif // SENTRYINPUT_H

#ifndef SENTRYINPUT_H
#define SENTRYINPUT_H

#include <QObject>
#include <QThread>

#include "parametermanager.h"

class Controller;

class SentryInput : public QThread, public ParameterOwner
{
    Q_OBJECT
public:
    explicit SentryInput(Controller *controller);
    void run();

    void init();

    ParameterManager *paramenterManager();
    QString settingsGroup();

protected:
    ParameterList createParameters() const;

private slots:
    void onParametersChanged();

private:
    Controller *m_controller;
    ParameterManager *m_parameterManager;

    int m_max_joy;
    qreal m_axis_body_x;
    qreal m_axis_body_y;
    qreal m_axis_laser_x;
    qreal m_axis_laser_y;

    char m_left_fire_button;
    char m_right_fire_button;
    char m_laser_button;

    char m_body_x_axis;
    char m_body_y_axis;
    char m_laser_x_axis;
    char m_laser_y_axis;
};

#endif // SENTRYINPUT_H

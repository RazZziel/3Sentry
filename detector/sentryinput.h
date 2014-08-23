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
    qreal m_axis_0;
    qreal m_axis_1;
    qreal m_axis_2;
    qreal m_axis_3;

    char m_left_fire_button;
    char m_right_fire_button;
    char m_laser_button;

};

#endif // SENTRYINPUT_H

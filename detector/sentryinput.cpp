#include "sentryinput.h"

#include <SDL2/SDL.h>

#include "controller.h"

#include <QDebug>
#include <QDateTime>

#define MAX_JOY 1500

SentryInput::SentryInput(Controller *controller) :
    QThread(controller),
    m_controller(controller),
    m_parameterManager(new ParameterManager(this, this)),
    cat("SentryInput"),
    m_dead_zone_radius(5)
{
    // Resources:
    //   https://code.google.com/p/joypick/

    //cat.setEnabled(QtDebugMsg, false);

    m_parameterManager->init();

    init();
}

void SentryInput::run()
{
    SDL_Event event;
    while (true)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_JOYDEVICEADDED:
            {
                SDL_Joystick *joystick = SDL_JoystickOpen(event.jdevice.which);
                if (joystick)
                {
                    m_joysticks.insert(SDL_JoystickInstanceID(joystick),
                                       new Joystick(joystick));
                }
                else
                {
                    qWarning() << "Could not open joystick" << event.jdevice.which;
                }
            }
                break;

            case SDL_JOYDEVICEREMOVED:
                qDebug() << "Removed joystick" << event.jdevice.which;

                if (m_joysticks.contains(event.jdevice.which))
                {
                    delete m_joysticks[event.jdevice.which];
                    m_joysticks.remove(event.jdevice.which);
                }
                else
                {
                    qWarning() << "But it does not exist?";
                }
                break;

            case SDL_JOYBUTTONDOWN:
            {
                Joystick *joystick = m_joysticks[event.jbutton.which];

                qDebug(cat) << "Pushed button" << event.jbutton.button;

                if (event.jbutton.button == joystick->m_button_left_fire)
                {
                    m_controller->startFiring(Hardware::LeftGun);
                }
                else if (event.jbutton.button == joystick->m_button_right_fire)
                {
                    m_controller->startFiring(Hardware::RightGun);
                }
                else if (event.jbutton.button == joystick->m_button_eyeLaser)
                {
                    m_controller->startFiring(Hardware::EyeLaser);
                }
                else if (event.jbutton.button == 4)
                {
                    m_controller->startFiring(Hardware::LeftLaser);
                }
                else if (event.jbutton.button == 5)
                {
                    m_controller->startFiring(Hardware::RightLaser);
                }
                else if (event.jbutton.button == 10)
                {
                    m_controller->hardware()->center(Hardware::Body);
                }
                else if (event.jbutton.button == 11)
                {
                    m_controller->hardware()->center(Hardware::Eye);
                }
            }
                break;

            case SDL_JOYBUTTONUP:
            {
                Joystick *joystick = m_joysticks[event.jbutton.which];

                bool doubleClick = (joystick->m_doubleClickTimers[event.jbutton.button].isValid() &&
                        joystick->m_doubleClickTimers[event.jbutton.button].msecsTo(QDateTime::currentDateTime()) <= 300);

                qDebug(cat) << "Released button" << event.jbutton.button
                            << (doubleClick
                                ? "(double click)"
                                : "");

                if (!doubleClick)
                {
                    if (event.jbutton.button == joystick->m_button_left_fire)
                    {
                        m_controller->stopFiring(Hardware::LeftGun);
                    }
                    else if (event.jbutton.button == joystick->m_button_right_fire)
                    {
                        m_controller->stopFiring(Hardware::RightGun);
                    }
                    else if (event.jbutton.button == joystick->m_button_eyeLaser)
                    {
                        m_controller->stopFiring(Hardware::EyeLaser);
                    }
                    else if (event.jbutton.button == 4)
                    {
                        m_controller->stopFiring(Hardware::LeftLaser);
                    }
                    else if (event.jbutton.button == 5)
                    {
                        m_controller->stopFiring(Hardware::RightLaser);
                    }
                }

                joystick->m_doubleClickTimers[event.jbutton.button] = QDateTime::currentDateTime();
            }
                break;

            case SDL_JOYAXISMOTION:
            {
                Joystick *joystick = m_joysticks[event.jbutton.which];

                qreal movement = 0;
                if (qAbs(event.jaxis.value) > m_dead_zone_radius)
                {
                    if (qAbs(event.jaxis.value) > joystick->m_max_joy)
                    {
                        joystick->m_max_joy = qAbs(event.jaxis.value);
                    }

                    movement = (qreal)event.jaxis.value / (qreal)joystick->m_max_joy;
                }

                qreal m_axis_body_x;
                qreal m_axis_body_y;
                qreal m_axis_laser_x;
                qreal m_axis_laser_y;

                if (event.jaxis.axis == joystick->m_axis_body_x)
                {
                    m_axis_body_x = movement;
                }
                else if (event.jaxis.axis == joystick->m_axis_body_y)
                {
                    m_axis_body_y = movement;
                }
                else if (event.jaxis.axis == joystick->m_axis_laser_x)
                {
                    m_axis_laser_x = movement;
                }
                else if (event.jaxis.axis == joystick->m_axis_laser_y)
                {
                    m_axis_laser_y = movement;
                }

                qDebug(cat) << event.jaxis.axis
                            << ":" << event.jaxis.value
                            << "=" << movement;

                if (event.jaxis.axis == 0 || event.jaxis.axis == 1)
                {
                    m_controller->targetRelative(Hardware::Body, m_axis_body_x, m_axis_body_y);
                }
                else
                {
                    m_controller->targetRelative(Hardware::Eye, m_axis_laser_x, m_axis_laser_y);
                }
            }
                break;
            }
        }
    }
}

void SentryInput::init()
{
    if (SDL_Init( SDL_INIT_JOYSTICK ) < 0)
    {
        qWarning(cat) << "Couldn't initialize SDL:" << SDL_GetError();
        return;
    }
}

ParameterManager *SentryInput::parameterManager()
{
    return m_parameterManager;
}

QString SentryInput::settingsGroup()
{
    return QString("Input");
}

ParameterList SentryInput::createParameters() const
{
    ParameterList list;
    list << Parameter("mapping/leftGunButton", tr("Left gun fire button"), Parameter::Integer, 6)
         << Parameter("mapping/rightGunButton", tr("Right gun fire button"), Parameter::Integer, 7)
         << Parameter("mapping/laserButton", tr("Laser fire button"), Parameter::Integer, 1)
         << Parameter("mapping/bodyHorizAxis", tr("Body horizontal axis"), Parameter::Integer, 0)
         << Parameter("mapping/bodyVertAxis", tr("Body vertical axis"), Parameter::Integer, 1)
         << Parameter("mapping/laserHorizAxis", tr("Laser horizontal axis"), Parameter::Integer, 2)
         << Parameter("mapping/laserVertAxis", tr("Laser vertical axis"), Parameter::Integer, 3)
         << Parameter("deadZoneRadius", tr("Joystick dead zone radius"), Parameter::Integer, 5);
    return list;
}

void SentryInput::onParametersChanged()
{
    m_dead_zone_radius = m_parameterManager->value("deadZoneRadius").toInt();
}


SentryInput::Joystick::Joystick(SDL_Joystick *joystick)
{
    char guid[255];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, 255);

    qDebug() << "Added joystick"
             << "[" << guid << "]"
             << SDL_JoystickName(joystick)
             << "with ID" << SDL_JoystickInstanceID(joystick);

    if (!strcmp(guid, "030000004c050000c405000011010000"))
    {
        // Sony Computer Entertainment Wireless Controller

        m_button_eyeLaser = 1;
        m_button_left_fire = 6;
        m_button_right_fire = 7;
        m_button_left_laser = 0; // ?
        m_button_right_laser = 0; // ?

        m_axis_body_x = 0;
        m_axis_body_y = 1;
        m_axis_laser_x = 2;
        m_axis_laser_y = 5;
    }
    else if (!strcmp(guid, "030000005e0400008e02000014010000"))
    {
        // Microsoft X-Box 360 pad

        m_button_eyeLaser = 5;
        m_button_left_fire = 6;
        m_button_right_fire = 4;
        m_button_left_laser = 0; // ?
        m_button_right_laser = 0; // ?

        m_axis_body_x = 0;
        m_axis_body_y = 1;
        m_axis_laser_x = 3;
        m_axis_laser_y = 4;
    }
    else // else if (!strcmp(guid, "030000006d04000018c2000010010000"))
    {
        // Logitech Logitech RumblePad 2 USB

        m_button_eyeLaser = 1;
        m_button_left_fire = 6;
        m_button_right_fire = 7;
        m_button_left_laser = 4; // ?
        m_button_right_laser = 5; // ?

        m_axis_body_x = 0;
        m_axis_body_y = 1;
        m_axis_laser_x = 2;
        m_axis_laser_y = 3;
    }

    m_max_joy = MAX_JOY;
}

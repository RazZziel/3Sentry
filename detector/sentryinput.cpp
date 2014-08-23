#include "sentryinput.h"

#include "SDL2/SDL.h"

#include "controller.h"

#include <QDebug>

#define MIN_JOY 3200
#define MAX_JOY 32000

SentryInput::SentryInput(Controller *controller) :
    QThread(controller),
    m_controller(controller)
{
    // Resources:
    //   https://code.google.com/p/joypick/

    m_left_fire_button = 6;
    m_right_fire_button = 7;
    m_laser_button = 1;

    init();
}

void SentryInput::run()
{
    SDL_Event event;
    while(true) {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_JOYBUTTONDOWN:
                qDebug() << "Pushed button" << event.jbutton.button;
                if(event.jbutton.button == m_left_fire_button)
                {
                    m_controller->enableFiring(Hardware::LeftGun);
                }
                else if(event.jbutton.button == m_right_fire_button)
                {
                    m_controller->enableFiring(Hardware::RightGun);
                }
                else if(event.jbutton.button == m_laser_button)
                {
                    m_controller->enableFiring(Hardware::EyeLaser);
                }
                break;

            case SDL_JOYBUTTONUP:
                qDebug() << "Released button" << event.jbutton.button;
                if(event.jbutton.button == m_left_fire_button)
                {
                    m_controller->stopFiring(Hardware::LeftGun);
                }
                if(event.jbutton.button == m_right_fire_button)
                {
                    m_controller->stopFiring(Hardware::RightGun);
                }
                if(event.jbutton.button == m_laser_button)
                {
                    m_controller->stopFiring(Hardware::EyeLaser);
                }
                break;

            case SDL_JOYAXISMOTION:  /* Handle Joystick Motion */
                if ( ( event.jaxis.value < -MIN_JOY ) || (event.jaxis.value > MIN_JOY ) )
                {
                    if(event.jaxis.value > m_max_joy) {
                        m_max_joy = event.jaxis.value;
                    } else if (-event.jaxis.value > m_max_joy) {
                        m_max_joy = -event.jaxis.value;
                    }

                    qreal movement = (qreal)event.jaxis.value / (qreal)m_max_joy;
                    qDebug() << event.jaxis.axis << ":" << event.jaxis.value << "=" << movement;
                    switch(event.jaxis.axis) {
                    case 0:
                        m_controller->targetRelative(Hardware::Eye, movement, 0);
                        break;
                    case 1:
                        m_controller->targetRelative(Hardware::Eye, 0, movement);
                        break;
                    case 2:
                        m_controller->targetRelative(Hardware::Body, movement, 0);
                        break;
                    case 3:
                        m_controller->targetRelative(Hardware::Body, 0, movement);
                        break;
                    }
                    break;
                } else {
                    switch(event.jaxis.axis) {
                    case 0:
                    case 1:
                        m_controller->targetRelative(Hardware::Eye, 0, 0);
                        break;
                    case 2:
                    case 3:
                        m_controller->targetRelative(Hardware::Body, 0, 0);
                        break;
                    }
                }
            }
        }
    }
}

void SentryInput::init()
{
    if (SDL_Init( SDL_INIT_JOYSTICK ) < 0)
    {
        qWarning() << "Couldn't initialize SDL:" << SDL_GetError();
        return;
    }

    qDebug() << SDL_NumJoysticks() << "controllers were found";

    for(int i=0; i<SDL_NumJoysticks(); ++i) {
        SDL_Joystick* j = SDL_JoystickOpen(i);
        qDebug() << "    " << SDL_JoystickName(j);
    }

    m_max_joy = MAX_JOY;
}

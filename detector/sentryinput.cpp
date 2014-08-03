#include "sentryinput.h"
#include "controller.h"

SentryInput::SentryInput(Controller *controller) :
    QObject(controller),
    m_controller(controller)
{
    // TODO: Place here SDL code to handle joystick input
    // Resources:
    //   https://code.google.com/p/joypick/
}

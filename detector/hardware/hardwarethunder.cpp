#include "hardwarethunder.h"
#include <usb.h>

HardwareThunder::HardwareThunder(QObject *parent) :
    Hardware(parent)
{
    usb_init();
    usb_find_busses();
    usb_find_devices();

    struct usb_bus *busses = usb_get_busses();

    for (struct usb_bus *bus = busses; bus; bus = bus->next)
    {
        struct usb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == 6465)
            {
                m_usbHandler = usb_open(dev);

                int claimed = usb_claim_interface(m_usbHandler, 0);
                if (claimed == 0)
                {
                    // TODO
                }
            }
        }
    }
}

HardwareThunder::~HardwareThunder()
{
    usb_release_interface(m_usbHandler, 0);
    usb_close(m_usbHandler);
}

bool HardwareThunder::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(minX);
    Q_UNUSED(maxX);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);
    return false;
}

bool HardwareThunder::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    return false;
}

bool HardwareThunder::targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(convertPos);
    return false;
}

bool HardwareThunder::targetRelative(Pantilt pantilt, int dx, int dy)
{
    Q_UNUSED(pantilt);

    if (dy > 0)
    {
        movement_handler(1);
    }
    else if (dy < 0)
    {
        movement_handler(2);
    }
    else if (dx < 0)
    {
        movement_handler(4);
    }
    else if (dx > 0)
    {
        movement_handler(8);
    }
    else
    {
        movement_handler(0);
    }

    return false;
}

bool HardwareThunder::enableFiring(Gun gun)
{
    Q_UNUSED(gun);
    movement_handler(16);
    return false;
}

bool HardwareThunder::stopFiring(Gun gun)
{
    Q_UNUSED(gun);
    movement_handler(0);
    return false;
}

int HardwareThunder::send_message(char* msg, int index)
{
    int ret = usb_control_msg(m_usbHandler, 0x21, 0x9, 0x200, index, msg, 8, 1000);

    //be sure that msg is all zeroes again
    msg[0] = 0x0;
    msg[1] = 0x0;
    msg[2] = 0x0;
    msg[3] = 0x0;
    msg[4] = 0x0;
    msg[5] = 0x0;
    msg[6] = 0x0;
    msg[7] = 0x0;

    return ret;
}

void HardwareThunder::movement_handler(char control)
{
    char msg[8];
    //reset
    msg[0] = 0x0;
    msg[1] = 0x0;
    msg[2] = 0x0;
    msg[3] = 0x0;
    msg[4] = 0x0;
    msg[5] = 0x0;
    msg[6] = 0x0;
    msg[7] = 0x0;

    //send 0s
    send_message(msg, 1);

    //send control
    msg[0] = control;
    send_message(msg, 0);

    //and more zeroes
    send_message(msg, 1);
}
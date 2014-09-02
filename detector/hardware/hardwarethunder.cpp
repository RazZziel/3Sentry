#include "hardwarethunder.h"
#include <usb.h>
#include <QDebug>
#include <QTimer>

HardwareThunder::HardwareThunder(QObject *parent) :
    Hardware(parent),
    m_usbHandler(NULL)
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
            if (dev->descriptor.idVendor == 0x2123 &&
                    dev->descriptor.idProduct == 0x1010)
            {
                qDebug() << "Found USB Raketenwerfer";

                m_usbHandler = usb_open(dev);
                if (m_usbHandler)
                {
                    int claimed = usb_claim_interface(m_usbHandler, 0);
                    if (claimed == 0)
                    {
                        qDebug() << "USB claimed";
                        // TODO
                        return;
                    }
                    else
                    {
                        qWarning() << "Error claiming USB interface:" << usb_strerror();
                    }
                }
                else
                {
                    qWarning() << "Could not open USB device:" << usb_strerror();
                }
            }
        }
    }

    qWarning() << "Could not find USB device";
}

HardwareThunder::~HardwareThunder()
{
    if (m_usbHandler)
    {
        usb_release_interface(m_usbHandler, 0);
        usb_close(m_usbHandler);
    }
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

bool HardwareThunder::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    if (pantilt == Body)
    {
        if (qAbs(dy) > qAbs(dx) && dy != 0)
        {
            movement_handler(2, dy < 0 ? 1 : 2);
        }
        else if (qAbs(dy) < qAbs(dx) && dx != 0)
        {
            movement_handler(2, dx < 0 ? 4 : 8);
        }
        else
        {
            movement_handler(2, 0);
        }
    }

    return true;
}

bool HardwareThunder::hw_startFiring(Trigger trigger)
{
    switch (trigger)
    {
    case EyeLaser:
        movement_handler(3, 1);
        break;
    case LeftGun:
    case RightGun:
        movement_handler(2, 0x10);
        break;
    case LeftLaser:
    case RightLaser:
        break;
    default:
        break;
    }

    return true;
}

bool HardwareThunder::hw_stopFiring(Trigger trigger)
{
    switch (trigger)
    {
    case EyeLaser:
        movement_handler(3, 0);
        break;
    case LeftGun:
    case RightGun:
        //movement_handler(2, 0);
    case LeftLaser:
    case RightLaser:
        //movement_handler(2, 0);
        break;
    default:
        break;
    }

    return true;
}

void HardwareThunder::movement_handler(char b0, char b1)
{
    if (!m_usbHandler)
    {
        qWarning() << "USB handler not initialized";
        return;
    }

    char msg[8];
    memset(msg, 0, sizeof(msg));
    msg[0] = b0;
    msg[1] = b1;

    usb_control_msg(m_usbHandler, 0x21, 0x9, 0, 0, msg, 8, 1000);
}

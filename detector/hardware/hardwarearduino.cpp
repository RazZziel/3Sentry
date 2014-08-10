#include "hardwarearduino.h"

HardwareArduino::HardwareArduino(QObject *parent) :
    Hardware(parent)
{
}

bool HardwareArduino::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(minX);
    Q_UNUSED(maxX);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);
    return false;
}

bool HardwareArduino::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    return false;
}

bool HardwareArduino::targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(convertPos);
    return false;
}

bool HardwareArduino::targetRelative(Pantilt pantilt, uint dx, uint dy)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(dx);
    Q_UNUSED(dy);
    return false;
}

bool HardwareArduino::enableFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

bool HardwareArduino::stopFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

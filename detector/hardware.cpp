#include "hardware.h"

Hardware::Hardware()
{
}

void Hardware::setCalibrationData(Pantilt pantilt, CalibrationData calibrationData)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(calibrationData);
}

bool Hardware::getLimits(Pantilt pantilt, uint &minX, uint &maxX, uint &minY, uint &maxY)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(minX);
    Q_UNUSED(maxX);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);
    return false;
}

bool Hardware::currentPosition(Pantilt pantilt, uint &x, uint &y)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    return false;
}

bool Hardware::target(uint pantilt, uint x, uint y)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    return false;
}

bool Hardware::enableFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

bool Hardware::stopFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

QPoint Hardware::calibratePoint(QPoint XYonCam)
{
    Q_UNUSED(XYonCam);
    return QPoint();
}

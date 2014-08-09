#include "hardware.h"

Hardware::Hardware(QObject *parent) :
    QObject(parent)
{
}

void Hardware::setCalibrationData(Pantilt pantilt, CalibrationData calibrationData)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(calibrationData);
}

QPoint Hardware::screen2hardware(QPoint xyOnScreen) const
{
    return xyOnScreen;
}

QPoint Hardware::hardware2screen(QPoint xyOnHardware) const
{
    return xyOnHardware;
}

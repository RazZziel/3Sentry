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
    QPoint xyOnHardware;

    // TODO
    xyOnHardware = QPoint(xyOnScreen.x()-100, xyOnScreen.y()-100);

    return xyOnHardware;
}

QPoint Hardware::hardware2screen(QPoint xyOnHardware) const
{
    QPoint xyOnScreen;

    // TODO
    xyOnScreen = QPoint(xyOnHardware.x()+100, xyOnHardware.y()+100);

    return xyOnScreen;
}

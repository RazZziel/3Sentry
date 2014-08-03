#ifndef HARDWARE_H
#define HARDWARE_H

#include <QMap>
#include <QPoint>
#include <QMatrix2x3>

class Hardware
{
public:
    Hardware();

    enum Pantilt {
        Body = 0,
        Eye = 1
    };

    enum Gun {
        EyeLaser = 0,
        RightGun = 1,
        LeftGun = 2
    };

    // CalibrationData[XYonHardware] = XYonCam
    // CalibrationData[XY'] = XY
    typedef QMap<QPoint,QPoint> CalibrationData;

    void setCalibrationData(Pantilt pantilt, CalibrationData calibrationData);

    bool currentPosition(Pantilt pantilt, uint &x, uint &y);
    bool targetAbsolute(Pantilt pantilt, uint x, uint y);
    bool targetRelative(Pantilt pantilt, uint dx, uint dy);
    bool enableFiring(Gun gun);
    bool stopFiring(Gun gun);

private:
    bool getLimits(Pantilt pantilt, uint &minX, uint &maxX, uint &minY, uint &maxY);
    QPoint calibratePoint(QPoint XYonCam);

    QMatrix2x3 m_calibrationMatrix;
};

#endif // HARDWARE_H

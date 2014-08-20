#ifndef HARDWARE_H
#define HARDWARE_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <QTransform>

class Hardware : public QObject
{
    Q_OBJECT
public:
    Hardware(QObject *parent=0);

    enum Pantilt {
        Body = 0,
        Eye = 1
    };

    enum Gun {
        EyeLaser = 0,
        RightGun = 1,
        LeftGun = 2
    };

    // CalibrationData <xyOnHardware, xyOnCam>
    // CalibrationData <XY,XY'>
    typedef QPair<QPoint,QPoint> PointPair;
    typedef QList< PointPair > CalibrationData;

    virtual bool getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY) =0;
    bool setCalibrationData(Pantilt pantilt, CalibrationData calibrationData);
    QTransform calibrationMatrix(Pantilt pantilt);

    virtual bool currentPosition(Pantilt pantilt, uint &x, uint &y) const =0;
    virtual bool targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos=true) =0;
    virtual bool targetRelative(Pantilt pantilt, int dx, int dy) =0;
    virtual bool enableFiring(Gun gun) =0;
    virtual bool stopFiring(Gun gun) =0;

protected:
    QPoint screen2hardware(Pantilt pantilt, QPoint xyOnScreen) const;
    QPoint hardware2screen(Pantilt pantilt, QPoint xyOnHardware) const;

    QMap<Pantilt,QTransform> m_calibrationMatrix;
    QMap<Pantilt,QTransform> m_calibrationMatrixInverted;

signals:
    void currentPositionChanged(Pantilt pantilt, int x, int y);
};

#endif // HARDWARE_H

#ifndef HARDWARE_H
#define HARDWARE_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <QTransform>
#include "parametermanager.h"

class Hardware : public QObject, public ParameterOwner
{
    Q_OBJECT
public:
    Hardware(QObject *parent=0);

    enum Pantilt {
        Body = 0,
        Eye = 1
    };

    enum Trigger {
        EyeLaser = 0,
        LeftGun = 1,
        RightGun = 2,
        LeftLaser = 3,
        RightLaser = 4,
        __Trigger_Size__
    };

    enum Speed {
        Slow = 0,
        Fast = 1
    };

    // CalibrationData <xyOnHardware, xyOnCam>
    // CalibrationData <XY,XY'>
    typedef QPair<QPoint,QPoint> PointPair;
    typedef QList<PointPair> CalibrationData;

    void init();
    QString settingsGroup();
    virtual ParameterList createParameters() const;
    ParameterManager *parameterManager();

    virtual bool getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY) =0;
    bool setCalibrationData(Pantilt pantilt, CalibrationData calibrationData);
    QTransform calibrationMatrix(Pantilt pantilt);

    virtual bool currentPosition(Pantilt pantilt, uint &x, uint &y) const =0;
    virtual bool targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos=true) =0;
    virtual bool targetRelative(Pantilt pantilt, qreal dx, qreal dy) =0;
    virtual bool center(Pantilt pantilt);
    virtual bool startFiring(Trigger trigger);
    virtual bool stopFiring(Trigger trigger);

    virtual Speed manualControlSpeed();
    virtual bool setManualControlSpeed(Speed speed);

protected:
    virtual bool hw_startFiring(Trigger trigger) =0;
    virtual bool hw_stopFiring(Trigger trigger) =0;

protected slots:
    virtual void onParametersChanged();

protected:
    QPoint screen2hardware(Pantilt pantilt, QPoint xyOnScreen) const;
    QPoint hardware2screen(Pantilt pantilt, QPoint xyOnHardware) const;

    QHash<Pantilt,QTransform> m_calibrationMatrix;
    QHash<Pantilt,QTransform> m_calibrationMatrixInverted;

    QHash<Trigger,QTimer*> m_firingTimer;
    QHash<Trigger,bool> m_hwIsFiring;

    Speed m_manualControlSpeed;

    ParameterManager *m_parameterManager;

private slots:
    void toggleTrigger();
    void toggleTrigger(Trigger trigger);

signals:
    void currentPositionChanged(Pantilt pantilt, int x, int y);
};

#endif // HARDWARE_H

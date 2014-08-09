#ifndef HARDWAREEMULATOR_H
#define HARDWAREEMULATOR_H

#include <QTimer>
#include <QEasingCurve>
#include "hardware.h"

class HardwareEmulator : public Hardware
{
    Q_OBJECT
public:
    HardwareEmulator(QObject *parent=0);

    bool currentPosition(Pantilt pantilt, uint &x, uint &y) const;
    bool targetAbsolute(Pantilt pantilt, uint x, uint y);
    bool targetRelative(Pantilt pantilt, uint dx, uint dy);
    bool enableFiring(Gun gun);
    bool stopFiring(Gun gun);

private:
    bool getLimits(Pantilt pantilt, uint &minX, uint &maxX, uint &minY, uint &maxY);

    QMap<Pantilt,QPoint> m_currentHwTarget;
    QMap<Pantilt,QPoint> m_currentHwPosition;
    QMap<Pantilt,qreal> m_pantiltSpeed;
    QMap<Gun,bool> m_firing;

    QTimer m_timer;

private slots:
    void tick();
};

#endif // HARDWAREEMULATOR_H

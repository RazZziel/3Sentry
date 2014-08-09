#ifndef HARDWAREARDUINO_H
#define HARDWAREARDUINO_H

#include "hardware.h"

class HardwareArduino : public Hardware
{
    Q_OBJECT
public:
    HardwareArduino(QObject *parent=0);

    bool currentPosition(Pantilt pantilt, uint &x, uint &y) const;
    bool targetAbsolute(Pantilt pantilt, uint x, uint y);
    bool targetRelative(Pantilt pantilt, uint dx, uint dy);
    bool enableFiring(Gun gun);
    bool stopFiring(Gun gun);

private:
    bool getLimits(Pantilt pantilt, uint &minX, uint &maxX, uint &minY, uint &maxY);
};

#endif // HARDWAREARDUINO_H

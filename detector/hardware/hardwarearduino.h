#ifndef HARDWAREARDUINO_H
#define HARDWAREARDUINO_H

#include "hardware.h"
#include <QMutex>

class QSerialPort;

class HardwareArduino : public Hardware
{
    Q_OBJECT
public:
    HardwareArduino(QObject *parent=0);
    ~HardwareArduino();

    ParameterList createParameters() const;

    bool getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY);
    bool currentPosition(Pantilt pantilt, uint &x, uint &y) const;
    bool targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos=true);
    bool targetRelative(Pantilt pantilt, qreal dx, qreal dy);
    bool startFiring(Gun gun);
    bool stopFiring(Gun gun);

private:
    bool sendCommand(const QByteArray &payload, QByteArray *ret_reply=NULL) const;
    QSerialPort *m_serialPort;
    mutable QMutex m_commandMutex;

private slots:
    void onParametersChanged();
};

#endif // HARDWAREARDUINO_H

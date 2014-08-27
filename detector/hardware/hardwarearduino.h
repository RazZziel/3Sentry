#ifndef HARDWAREARDUINO_H
#define HARDWAREARDUINO_H

#include "hardware.h"
#include <QMutex>
#include <QTimer>

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
    bool center(Pantilt pantilt);
    bool startFiring(Trigger trigger);
    bool stopFiring(Trigger trigger);

private:
    bool hw_getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY);
    bool hw_currentPosition(Pantilt pantilt, uint &x, uint &y) const;

    bool sendCommand(const QByteArray &payload, QByteArray *ret_reply=NULL) const;
    bool readReply(QByteArray *ret_reply) const;

    QSerialPort *m_serialPort;
    mutable QMutex m_commandMutex;
    QMap<Pantilt,QPointF> m_currentHwPosition;
    QMap<Pantilt,QPointF> m_minHwPosition;
    QMap<Pantilt,QPointF> m_maxHwPosition;
    QMap<Pantilt,QPointF> m_pantiltSpeed;
    QMap<Pantilt,QPointF> m_pantiltAcc;

    QTimer m_positionQueryTimer;
    QTimer m_positionUpdateTimer;

private slots:
    void onParametersChanged();
    void updateCurrentPosition();
    void sendCurrentPosition();
};

#endif // HARDWAREARDUINO_H

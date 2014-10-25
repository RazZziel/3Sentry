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
    bool currentDistance(Pantilt pantilt, uint &distance) const;
    bool targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos=true);
    bool targetRelative(Pantilt pantilt, qreal dx, qreal dy);
    bool center(Pantilt pantilt);
    bool setManualControlSpeed(Speed speed);

private:
    bool hw_targetAbsolute(Pantilt pantilt, uint x, uint y);
    bool hw_updateLimits(Pantilt pantilt);
    bool hw_startFiring(Trigger trigger);
    bool hw_stopFiring(Trigger trigger);

    bool hw_updateCurrentPosition(Pantilt pantilt);
    bool hw_updateDistance(Pantilt pantilt);

    bool sendCommand(const QByteArray &payload, QByteArray *ret_reply=NULL) const;
    bool readReply(QByteArray *ret_reply) const;

    QSerialPort *m_serialPort;
    mutable QMutex m_commandMutex;
    QMap<Pantilt,QPoint> m_currentHwPosition;
    QMap<Pantilt,uint> m_currentHwDistance;
    QMap<Pantilt,QPointF> m_wantedHwPosition;
    QMap<Pantilt,QPointF> m_pantiltCurrentSpeed;
    QMap<Pantilt,QPointF> m_minHwPosition;
    QMap<Pantilt,QPointF> m_maxHwPosition;
    QMap<Pantilt,QPointF> m_pantiltAcc;

    QTimer m_positionQueryTimer;
    QTimer m_positionUpdateTimer;

private slots:
    void onParametersChanged();
    void updateCurrentPosition();
    void sendCurrentPosition();
};

#endif // HARDWAREARDUINO_H

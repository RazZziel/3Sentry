#include "hardwarearduino.h"
#include <QtSerialPort/QtSerialPort>
#include <QCoreApplication>

static int speedMultiplier_Slow = 5;
static int speedMultiplier_Fast = 10;

HardwareArduino::HardwareArduino(QObject *parent) :
    Hardware(parent),
    m_serialPort(new QSerialPort(this))
{
    connect(&m_positionQueryTimer, SIGNAL(timeout()), SLOT(updateCurrentPosition()));
    m_positionQueryTimer.setInterval(10*1000);

    connect(&m_positionUpdateTimer, SIGNAL(timeout()), SLOT(sendCurrentPosition()));
    m_positionUpdateTimer.setInterval(16);

    m_pantiltCurrentSpeed[Eye] = QPointF(0, 0);
    m_pantiltCurrentSpeed[Body] = QPointF(0, 0);

    setManualControlSpeed(Slow);
}

HardwareArduino::~HardwareArduino()
{
    m_serialPort->close();
}

ParameterList HardwareArduino::createParameters() const
{
    QVariantMap serialPorts;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString name = QString("%1 (%2)").arg(info.description()).arg(info.portName());
        serialPorts.insert(name, info.systemLocation());
    }

    ParameterList list = Hardware::createParameters();
    list << Parameter::selection("portName", tr("Port name"), serialPorts);
    return list;
}

void HardwareArduino::onParametersChanged()
{
    Hardware::onParametersChanged();

    QString newPortName = m_parameterManager->value("portName").toString();
    if (m_serialPort->portName() != newPortName)
    {
        m_serialPort->close();
        m_serialPort->setPortName(newPortName);

        m_serialPort->setBaudRate(QSerialPort::Baud19200);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setStopBits(QSerialPort::OneStop);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (!m_serialPort->open(QIODevice::ReadWrite))
        {
            qWarning() << "Could not open serial port" << m_serialPort->portName()
                       << ":" << m_serialPort->errorString();
        }
        else
        {
            hw_updateLimits(Body);
            hw_updateLimits(Eye);
            updateCurrentPosition();
        }
    }

    /*if (m_parameterManager->value("monitorPosition").toBool())
    {
        m_positionQueryTimer.start();
    }
    else
    {
        m_positionQueryTimer.stop();
    }*/
    m_positionQueryTimer.start();
    m_positionUpdateTimer.start();
}

void HardwareArduino::updateCurrentPosition()
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;

        if (hw_updateCurrentPosition(pantilt))
        {
            QPoint screeenPoint = hardware2screen(pantilt, m_currentHwPosition[pantilt]);
            emit currentPositionChanged(pantilt, screeenPoint.x(), screeenPoint.y());

            hw_updateDistance(pantilt);
        }
        else
        {
            qWarning() << "Could not update pantilt position";
        }
    }
}

void HardwareArduino::sendCurrentPosition()
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;

        //qreal dx = qPow(m_pantiltCurrentSpeed[pantilt].x(), 3) * m_pantiltAcc[pantilt].x();
        //qreal dy = qPow(m_pantiltCurrentSpeed[pantilt].y(), 3) * m_pantiltAcc[pantilt].y();
        qreal dx = m_pantiltCurrentSpeed[pantilt].x() * m_pantiltAcc[pantilt].x();
        qreal dy = m_pantiltCurrentSpeed[pantilt].y() * m_pantiltAcc[pantilt].y();

        QPointF newHwPositionF = m_wantedHwPosition[pantilt] + QPointF(dx, dy);

        newHwPositionF.rx() = qMax(m_minHwPosition[pantilt].x(), newHwPositionF.x());
        newHwPositionF.rx() = qMin(m_maxHwPosition[pantilt].x(), newHwPositionF.x());
        newHwPositionF.ry() = qMax(m_minHwPosition[pantilt].y(), newHwPositionF.y());
        newHwPositionF.ry() = qMin(m_maxHwPosition[pantilt].y(), newHwPositionF.y());

        m_wantedHwPosition[pantilt] = newHwPositionF;

        QPoint newHwPosition = newHwPositionF.toPoint();
        if (m_currentHwPosition[pantilt] != newHwPosition)
        {
            if (hw_targetAbsolute(pantilt,
                                  newHwPosition.x(),
                                  newHwPosition.y()))
            {
                m_currentHwPosition[pantilt] = newHwPosition;
            }

            QPoint screenPosition = hardware2screen(pantilt, m_currentHwPosition[pantilt]);
            emit currentPositionChanged(pantilt, screenPosition.x(), screenPosition.y());

            qDebug() << "Pantilt" << pantilt
                     << "targeting:"
                     << "hw=" << newHwPositionF
                     << "screen=" << screenPosition;
        }
    }
}

bool HardwareArduino::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    minX = m_minHwPosition[pantilt].x();
    minY = m_minHwPosition[pantilt].y();
    maxX = m_maxHwPosition[pantilt].x();
    maxY = m_maxHwPosition[pantilt].y();
    return true;
}


bool HardwareArduino::hw_targetAbsolute(Pantilt pantilt, uint x, uint y)
{
    QByteArray payload("A");
    payload.append((quint8) pantilt);
    payload.append((quint8) x);
    payload.append((quint8) y);
    return sendCommand(payload);
}

bool HardwareArduino::hw_updateLimits(Pantilt pantilt)
{
    QByteArray payload("L");
    QByteArray reply;
    payload.append((quint8) pantilt);
    if (!sendCommand(payload, &reply))
    {
        return false;
    }

    if (reply.length() != 4)
    {
        qWarning() << "Invalid reply" << reply.toHex();
        return false;
    }

    m_minHwPosition[pantilt] = QPoint((quint8) reply[0], (quint8) reply[2]);
    m_maxHwPosition[pantilt] = QPoint((quint8) reply[1], (quint8) reply[3]);

    qDebug() << "Pantilt" << pantilt
             << "limits:"
             << "min" << m_minHwPosition[pantilt]
             << "max" << m_maxHwPosition[pantilt];

    return true;
}

bool HardwareArduino::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    x = m_currentHwPosition[pantilt].x();
    y = m_currentHwPosition[pantilt].y();
    return true;
}

bool HardwareArduino::currentDistance(Hardware::Pantilt pantilt, uint &distance) const
{
    distance = m_currentHwDistance[pantilt];
    return true;
}

bool HardwareArduino::hw_updateCurrentPosition(Pantilt pantilt)
{
    QByteArray payload("P");
    QByteArray reply;
    payload.append((quint8) pantilt);
    if (!sendCommand(payload, &reply))
    {
        return false;
    }

    if (reply.length() != 2)
    {
        qWarning() << "Invalid reply" << reply.toHex();
        return false;
    }

    QPoint hardwarePoint((quint8) reply[0], (quint8) reply[1]);
    QPoint screeenPoint = hardware2screen(pantilt, hardwarePoint);

    qDebug() << "Pantilt" << pantilt
             << "current position:"
             << "hw=" << hardwarePoint
             << "screen=" << screeenPoint;

    m_currentHwPosition[pantilt] = QPoint(hardwarePoint.x(), hardwarePoint.y());

    return true;
}

bool HardwareArduino::hw_updateDistance(Hardware::Pantilt pantilt)
{
    if (pantilt != Eye)
        return false;

    QByteArray payload("D");
    QByteArray reply;
    if (!sendCommand(payload, &reply))
    {
        return false;
    }

#if 0
    if (reply.length() != 2)
    {
        qWarning() << "Invalid reply" << reply.toHex();
        return false;
    }

    qDebug() << (quint8) reply[0] <<  (quint8) reply[1];
    uint distance = ( (quint8) reply[0] )*16 + ( (quint8) reply[1] );
#else
    if (reply.length() != 1)
    {
        qWarning() << "Invalid reply" << reply.toHex();
        return false;
    }

    uint distance = (quint8) reply[0];
#endif

    qDebug() << "Pantilt" << pantilt
             << "current distance=" << distance;

    m_currentHwDistance[pantilt] = distance;

    return true;
}

bool HardwareArduino::targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos)
{
    QPoint hwPosition(x, y);
    QPoint screenPosition(x, y);

    if (convertPos)
    {
        hwPosition = screen2hardware(pantilt, screenPosition);
    }
    else
    {
        screenPosition = hardware2screen(pantilt, hwPosition);
    }

    m_wantedHwPosition[pantilt] = hwPosition;

    return true;
}

bool HardwareArduino::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    m_pantiltCurrentSpeed[pantilt] = QPointF(dx, dy);
    return true;
}

bool HardwareArduino::center(Pantilt pantilt)
{
    return targetAbsolute(pantilt, 90, 90);
}

bool HardwareArduino::hw_startFiring(Trigger trigger)
{
    QByteArray payload("S");
    payload.append((quint8) trigger);
    return sendCommand(payload);
}

bool HardwareArduino::hw_stopFiring(Trigger trigger)
{
    QByteArray payload("H");
    payload.append((quint8) trigger);
    return sendCommand(payload);
}

bool HardwareArduino::setManualControlSpeed(Speed speed)
{
    Hardware::setManualControlSpeed(speed);

    switch (speed)
    {
    case Slow:
        m_pantiltAcc[Eye] = QPointF(-speedMultiplier_Slow, speedMultiplier_Slow);
        m_pantiltAcc[Body] = QPointF(-speedMultiplier_Slow, speedMultiplier_Slow);
        break;
    case Fast:
        m_pantiltAcc[Eye] = QPointF(-speedMultiplier_Fast, speedMultiplier_Fast);
        m_pantiltAcc[Body] = QPointF(-speedMultiplier_Fast, speedMultiplier_Fast);
        break;
    }

    return true;
}

bool HardwareArduino::sendCommand(const QByteArray &payload, QByteArray *ret_reply) const
{
    QMutexLocker ml(&m_commandMutex);

    int retries = 3;
    bool gotReply = false;
    do
    {
        if (ret_reply)
        {
            m_serialPort->clear();
        }

        QByteArray data;
        data.append((quint8) 0x02);
        data.append(payload);
        data.append((quint8) 0x03);

        qDebug() << "<<" << payload[0] << data.toHex();

        qint64 written = m_serialPort->write(data);
        if (written != data.length())
        {
            qWarning() << "Could not write to serial port, written" << written
                       << "instead of" << data.length();
            return false;
        }

        if (!ret_reply)
        {
            return true;
        }
        else
        {
            gotReply = readReply(ret_reply);
        }
    }
    while (retries-- > 0 && !gotReply);

    return gotReply;
}

bool HardwareArduino::readReply(QByteArray *ret_reply) const
{
    QByteArray reply;
    while (true)
    {
        if (!m_serialPort->waitForReadyRead(100))
        {
            qWarning() << "Could not read reply";

            if (!reply.isEmpty())
            {
                qWarning() << "After reading" << reply.toHex() << reply;
            }
            return false;
        }

        char c = 0;
        while (m_serialPort->getChar(&c))
        {
            if (c == 0x02)
            {
                reply.clear();
            }

            reply.append(c);

            if (c == 0x03)
            {
                qDebug() << ">>" << reply.toHex();

                if (ret_reply)
                {
                    *ret_reply = reply.mid(1, reply.length()-2);
                }

                return true;
            }
        }
    }

    return false; // Impossibru
}

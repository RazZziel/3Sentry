#include "hardwarearduino.h"
#include <QtSerialPort/QtSerialPort>
#include <QCoreApplication>

HardwareArduino::HardwareArduino(QObject *parent) :
    Hardware(parent),
    m_serialPort(new QSerialPort(this))
{
    connect(m_parameterManager, SIGNAL(parametersChanged()), SLOT(onParametersChanged()));
    onParametersChanged();

    connect(&m_positionUpdateTimer, SIGNAL(timeout()), SLOT(sendCurrentPosition()));
    m_positionUpdateTimer.setInterval(32);

    connect(&m_positionQueryTimer, SIGNAL(timeout()), SLOT(updateCurrentPosition()));
    m_positionQueryTimer.setInterval(10*1000);

    m_pantiltSpeed[Eye] = QPointF(0, 0);
    m_pantiltSpeed[Body] = QPointF(0, 0);
    m_pantiltAcc[Eye] = QPointF(-30, 30);
    m_pantiltAcc[Body] = QPointF(-30, 30);
}

HardwareArduino::~HardwareArduino()
{
    m_serialPort->close();
}

ParameterList HardwareArduino::createParameters() const
{
    ParameterList list = Hardware::createParameters();
    list << Parameter("portName", tr("Port name"), Parameter::String, "/dev/ttyUSB0");
    return list;
}

void qSleep(float seconds)
{
    // Sucks to be me
    QDateTime sleepTime = QDateTime::currentDateTime().addMSecs(lround(seconds*1000));
    while (QDateTime::currentDateTime() < sleepTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
void HardwareArduino::onParametersChanged()
{
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
            qWarning() << "Could not open serial port:" << m_serialPort->errorString();
        }
        else
        {
            int minX, maxX, minY, maxY;
            getLimits(Body, minX, maxX, minY, maxY);
            getLimits(Eye, minX, maxX, minY, maxY);
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

        uint x=0;
        uint y=0;
        if (currentPosition(pantilt, x, y))
        {
            m_currentHwPosition[pantilt].rx() = x;
            m_currentHwPosition[pantilt].ry() = y;

            QPoint screeenPoint = hardware2screen(pantilt, m_currentHwPosition[pantilt].toPoint());
            emit currentPositionChanged(pantilt, screeenPoint.x(), screeenPoint.y());
        }
    }
}

void HardwareArduino::sendCurrentPosition()
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;

        qreal dx = qPow(m_pantiltSpeed[pantilt].x(), 3)*m_pantiltAcc[pantilt].x();
        qreal dy = qPow(m_pantiltSpeed[pantilt].y(), 3)*m_pantiltAcc[pantilt].y();

        QPointF newPosition = m_currentHwPosition[pantilt] + QPointF(dx, dy);

        newPosition.rx() = qMax(m_minHwPosition[pantilt].x(), newPosition.x());
        newPosition.rx() = qMin(m_maxHwPosition[pantilt].x(), newPosition.x());
        newPosition.ry() = qMax(m_minHwPosition[pantilt].y(), newPosition.y());
        newPosition.ry() = qMin(m_maxHwPosition[pantilt].y(), newPosition.y());

        targetAbsolute(pantilt,
                       newPosition.x(),
                       newPosition.y(), false);
    }
}

bool HardwareArduino::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
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

    minX = (quint8) reply[0];
    maxX = (quint8) reply[1];
    minY = (quint8) reply[2];
    maxY = (quint8) reply[3];

    qDebug() << "Limits:" << minX << maxX << minY << maxY;

    m_minHwPosition[pantilt] = QPoint(minX, minY);
    m_maxHwPosition[pantilt] = QPoint(maxX, maxY);

    return true;
}

bool HardwareArduino::currentPosition(Pantilt pantilt, uint &x, uint &y) const
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
    x = screeenPoint.x();
    y = screeenPoint.y();

    qDebug() << "Current position:" << x << y;

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

    if (hwPosition != m_currentHwPosition[pantilt].toPoint())
    {
        emit currentPositionChanged(pantilt, screenPosition.x(), screenPosition.y());

        m_currentHwPosition[pantilt] = hwPosition;

        QByteArray payload("A");
        payload.append((quint8) pantilt);
        payload.append((quint8) hwPosition.x());
        payload.append((quint8) hwPosition.y());
        return sendCommand(payload);
    }
    else
    {
        return true;
    }
}

bool HardwareArduino::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    m_pantiltSpeed[pantilt] = QPointF(dx, dy);
    return true;
}

bool HardwareArduino::startFiring(Trigger trigger)
{
    QByteArray payload("S");
    payload.append((quint8) trigger);
    return sendCommand(payload);
}

bool HardwareArduino::stopFiring(Trigger trigger)
{
    QByteArray payload("H");
    payload.append((quint8) trigger);
    return sendCommand(payload);
}

bool HardwareArduino::sendCommand(const QByteArray &payload, QByteArray *ret_reply) const
{
    QMutexLocker ml(&m_commandMutex);

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

    if (ret_reply)
    {
        if (!readReply(ret_reply))
        {
            return false;
        }
    }

    return true;
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

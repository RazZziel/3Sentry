#include "hardwarearduino.h"
#include <QtSerialPort/QtSerialPort>

HardwareArduino::HardwareArduino(QObject *parent) :
    Hardware(parent),
    m_serialPort(new QSerialPort(this))
{
    connect(m_parameterManager, SIGNAL(parametersChanged()), SLOT(onParametersChanged()));
    onParametersChanged();
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
    }
}

bool HardwareArduino::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    QByteArray payload("L");
    payload.append((uchar) pantilt);
    QByteArray reply;
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

    return false;
}

bool HardwareArduino::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    QByteArray payload("P");
    payload.append((uchar) pantilt);
    QByteArray reply;
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
    if (convertPos)
    {
        QPoint hwPosition = screen2hardware(pantilt, QPoint(x,y));
        x = hwPosition.x();
        y = hwPosition.y();
    }

    QByteArray payload("A");
    payload.append((quint8) pantilt);
    payload.append((quint8) x);
    payload.append((quint8) x);
    return sendCommand(payload);
}

bool HardwareArduino::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    QByteArray payload("M");
    payload.append((quint8) pantilt);
    payload.append((quint8) qRound((dx * 127)+127));
    payload.append((quint8) qRound((dy * 127)+127));
    return sendCommand(payload);
}

bool HardwareArduino::startFiring(Gun gun)
{
    QByteArray payload("S");
    payload.append((quint8) gun);
    return sendCommand(payload);
}

bool HardwareArduino::stopFiring(Gun gun)
{
    QByteArray payload("H");
    payload.append((quint8) gun);
    return sendCommand(payload);
}

bool HardwareArduino::sendCommand(const QByteArray &payload, QByteArray *ret_reply) const
{
    QMutexLocker ml(&m_commandMutex);

    QByteArray data;
    data.append((uchar) 0x02);
    data.append(payload);
    data.append((uchar) 0x03);

    qDebug() << "<<" << payload[0] << data.toHex();

    qint64 written = m_serialPort->write(data);
    if (written != data.length())
    {
        qWarning() << "Could not write to serial port, written" << written
                   << "instead of" << data.length();
        return false;
    }

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

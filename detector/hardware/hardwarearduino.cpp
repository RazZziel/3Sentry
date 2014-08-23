#include "hardwarearduino.h"
#include <QtSerialPort/QtSerialPort>

HardwareArduino::HardwareArduino(QObject *parent) :
    Hardware(parent),
    m_serialPort(new QSerialPort(this))
{
    m_serialPort->setBaudRate(9600);
    m_serialPort->setPortName("/dev/ttyUSB0"); // TODO: Make configurable
    if (!m_serialPort->open(QIODevice::ReadWrite))
    {
        qWarning() << "Could not open serial port:" << m_serialPort->errorString();
    }
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

bool HardwareArduino::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(minX);
    Q_UNUSED(maxX);
    Q_UNUSED(minY);
    Q_UNUSED(maxY);
    return false;
}

bool HardwareArduino::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    return false;
}

bool HardwareArduino::targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(convertPos);
    return false;
}

bool HardwareArduino::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    Q_UNUSED(pantilt);
    Q_UNUSED(dx);
    Q_UNUSED(dy);
    return false;
}

bool HardwareArduino::startFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

bool HardwareArduino::stopFiring(Gun gun)
{
    Q_UNUSED(gun);
    return false;
}

bool HardwareArduino::sendCommand(const QByteArray &ba)
{
    QByteArray data;
    data.append(ba); // TODO: Prefixes, checksums, etc

    qDebug() << "<<" << data.toHex();

    if (m_serialPort->write(ba) != data.length())
    {
        qWarning() << "Could not write to serial port";
        return false;
    }

    QByteArray reply = m_serialPort->readAll().trimmed();
    if (reply.isEmpty())
    {
        qWarning() << "Could not read reply";
        return false;
    }

    qDebug() << ">>" << reply.toHex();

    // TODO: Do something with the reply, maybe

    return true;
}

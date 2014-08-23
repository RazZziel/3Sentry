#include "hardwareemulator.h"
#include <QDebug>

HardwareEmulator::HardwareEmulator(QObject *parent) :
    Hardware(parent)
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;
        m_currentHwTarget[pantilt] = QPoint(0,0);
        m_currentHwPosition[pantilt] = QPoint(0,0);
    }

    for (int i=EyeLaser; i<=LeftGun; i++)
    {
        Gun gun = (Gun) i;
        m_firing[gun] = false;
    }

    m_pantiltSpeed[Body] = 0.1;
    m_pantiltSpeed[Eye] = 0.2;

    connect(&m_timer, SIGNAL(timeout()), SLOT(tick()));
    m_timer.start(50);
}

bool HardwareEmulator::getLimits(Pantilt pantilt, int &minX, int &maxX, int &minY, int &maxY)
{
    Q_UNUSED(pantilt);
    minX=-45;
    maxX=45;
    minY=-45;
    maxY=45;
    return true;
}

bool HardwareEmulator::currentPosition(Pantilt pantilt, uint &x, uint &y) const
{
    QPoint screeenPoint = hardware2screen(pantilt, m_currentHwPosition[pantilt].toPoint());
    x = screeenPoint.x();
    y = screeenPoint.y();
    return true;
}

bool HardwareEmulator::targetAbsolute(Pantilt pantilt, uint x, uint y, bool convertPos)
{
    if (convertPos)
        m_currentHwTarget[pantilt] = screen2hardware(pantilt, QPoint(x,y));
    else
        m_currentHwTarget[pantilt] = QPoint(x,y);

    return true;
}

bool HardwareEmulator::targetRelative(Pantilt pantilt, qreal dx, qreal dy)
{
    m_currentHwTarget[pantilt].rx() += dx;
    m_currentHwTarget[pantilt].ry() += dy;

    return true;
}

bool HardwareEmulator::startFiring(Gun gun)
{
    if (!m_firing[gun])
    {
        m_firing[gun] = true;
        qDebug() << "Start firing" << gun;
    }
    return true;
}

bool HardwareEmulator::stopFiring(Gun gun)
{
    if (m_firing[gun])
    {
        m_firing[gun] = false;
        qDebug() << "Stop firing" << gun;
    }
    return true;
}

void HardwareEmulator::tick()
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;

        m_currentHwPosition[pantilt].setX(m_currentHwPosition[pantilt].x() +
                                          (qreal)(m_currentHwTarget[pantilt].x() -
                                                  m_currentHwPosition[pantilt].x()) * m_pantiltSpeed[pantilt]);
        m_currentHwPosition[pantilt].setY(m_currentHwPosition[pantilt].y() +
                                          (qreal)(m_currentHwTarget[pantilt].y() -
                                                  m_currentHwPosition[pantilt].y()) * m_pantiltSpeed[pantilt]);

        QPoint screeenPoint = hardware2screen(pantilt, m_currentHwPosition[pantilt].toPoint());
        emit currentPositionChanged(pantilt, screeenPoint.x(), screeenPoint.y());
    }
}

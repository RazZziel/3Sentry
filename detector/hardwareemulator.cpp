#include "hardwareemulator.h"

HardwareEmulator::HardwareEmulator(QObject *parent) :
    Hardware(parent)
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;
        m_currentHwTarget[pantilt] = QPoint(0,0);
        m_currentHwPosition[pantilt] = QPoint(0,0);
    }

    m_pantiltSpeed[Body] = 0.1;
    m_pantiltSpeed[Eye] = 0.2;

    connect(&m_timer, SIGNAL(timeout()), SLOT(tick()));
    m_timer.start(50);
}

bool HardwareEmulator::getLimits(Pantilt pantilt, uint &minX, uint &maxX, uint &minY, uint &maxY)
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
    QPoint screeenPoint = hardware2screen(m_currentHwPosition[pantilt]);
    x = screeenPoint.x();
    y = screeenPoint.y();
    return true;
}

bool HardwareEmulator::targetAbsolute(Pantilt pantilt, uint x, uint y)
{
    m_currentHwTarget[pantilt] = screen2hardware(QPoint(x,y));

    return true;
}

bool HardwareEmulator::targetRelative(Pantilt pantilt, uint dx, uint dy)
{
    m_currentHwTarget[pantilt].rx() += dx;
    m_currentHwTarget[pantilt].ry() += dy;

    return true;
}

bool HardwareEmulator::enableFiring(Gun gun)
{
    m_firing[gun] = true;
    return true;
}

bool HardwareEmulator::stopFiring(Gun gun)
{
    m_firing[gun] = false;
    return true;
}

void HardwareEmulator::tick()
{
    for (int i=Body; i<=Eye; i++)
    {
        Pantilt pantilt = (Pantilt) i;

        m_currentHwPosition[pantilt].setX(qRound(m_currentHwPosition[pantilt].x() +
                                                 (qreal)(m_currentHwTarget[pantilt].x() -
                                                         m_currentHwPosition[pantilt].x()) * m_pantiltSpeed[pantilt]));
        m_currentHwPosition[pantilt].setY(qRound(m_currentHwPosition[pantilt].y() +
                                                 (qreal)(m_currentHwTarget[pantilt].y() -
                                                         m_currentHwPosition[pantilt].y()) * m_pantiltSpeed[pantilt]));

        QPoint screeenPoint = hardware2screen(m_currentHwPosition[pantilt]);
        emit currentPositionChanged(pantilt, screeenPoint.x(), screeenPoint.y());
    }
}

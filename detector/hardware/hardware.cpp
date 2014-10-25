#include "hardware.h"
#include <QMatrix>
#include <QTimer>
#include <QDebug>
#include <cv.h>

Hardware::Hardware(QObject *parent) :
    QObject(parent),
    m_parameterManager(new ParameterManager(this, this))
{
    qRegisterMetaType<Pantilt>("Pantilt");
    connect(m_parameterManager, SIGNAL(parametersChanged()), SLOT(onParametersChanged()));

    for (int i=Hardware::EyeLaser; i<Hardware::__Trigger_Size__; i++)
    {
        Hardware::Trigger trigger = (Hardware::Trigger) i;
        m_triggers[trigger] = new TehTrigger(trigger, this);
    }
}

void Hardware::init()
{
    m_parameterManager->init();

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
        center(pantilt);
    }
}

QString Hardware::settingsGroup()
{
    return QString("Hardware/%1/").arg(metaObject()->className());
}

ParameterList Hardware::createParameters() const
{
    ParameterList list;
    list << Parameter(QString("pantilt/%1/calibration").arg(Eye), tr("Eye calibration matrix"), Parameter::Unknown, "")
         << Parameter(QString("pantilt/%1/calibration").arg(Body), tr("Body calibration matrix"), Parameter::Unknown, "");

    QVariantMap fireModes;
    fireModes.insert(tr("Pulse"), Pulse);
    fireModes.insert(tr("Fully automatic"), FullyAutomatic);
    fireModes.insert(tr("Burst"), Burst);
    fireModes.insert(tr("Single shot"), SingleShot);
    fireModes.insert(tr("Toggle"), Toggle);

    for (int i=Hardware::EyeLaser; i<Hardware::__Trigger_Size__; i++)
    {
        list << Parameter::selection(QString("trigger/%1/mode").arg(i),
                                     tr("Trigger %1 mode").arg(i),
                                     fireModes, Pulse);
    }

    return list;
}

ParameterManager *Hardware::parameterManager()
{
    return m_parameterManager;
}

void Hardware::onParametersChanged()
{
    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Pantilt pantilt = (Pantilt) i;
        m_calibrationMatrix[pantilt] = m_parameterManager->value(QString("calibration/%1").arg(pantilt)).value<QTransform>();
        m_calibrationMatrixInverted[pantilt] = m_calibrationMatrix[pantilt].inverted();
    }

    for (int i=Hardware::EyeLaser; i<Hardware::__Trigger_Size__; i++)
    {
        Trigger trigger = (Trigger) i;
        m_triggers[trigger]->setFireMode((FireMode) m_parameterManager->value(QString("trigger/%1/mode").arg(trigger)).toInt());
    }
}

bool Hardware::center(Pantilt pantilt)
{
    int minX, maxX, minY, maxY;
    if (!getLimits(pantilt, minX, maxX, minY, maxY))
    {
        qWarning() << "Could not get limits";
        return false;
    }

    return targetAbsolute(pantilt, (minX+maxX)/2, (minY+maxY)/2, false);
}

bool Hardware::startFiring(Trigger trigger)
{
    return m_triggers[trigger]->startFiring();
}

bool Hardware::stopFiring(Trigger trigger)
{
    return m_triggers[trigger]->stopFiring();
}

bool Hardware::setCalibrationData(Pantilt pantilt, CalibrationData calibrationData)
{
    // XY = xyOnHardware
    // XY' = xyOnScreen

#if 1 // OpenCV, N points
    int nPoints = calibrationData.length();
    float cvAdata[nPoints*3];
    float cvACdata[nPoints*2];
    int row=0;
    foreach (const PointPair &pair, calibrationData)
    {
        // A = (X'1 Y'1 1,
        //      X'2 Y'2 1,
        //      X'3 Y'3 1)
        cvAdata[row*3+0] = pair.first.x();
        cvAdata[row*3+1] = pair.first.y();
        cvAdata[row*3+2] = 1;

        // (ax bx cx)t = 1/A * (X1 X2 X3)t
        cvACdata[row*2+0] = pair.second.x();

        // (ay by cy)t = 1/A * (Y1 Y2 Y3)t
        cvACdata[row*2+1] = pair.second.y();

        ++row;
    }
    cv::Mat A(nPoints,3,CV_32F,cvAdata);
    cv::Mat C(nPoints,2,CV_32F,cvACdata);
    std::cout << "A " << A << std::endl;
    std::cout << "C " << C << std::endl;
    std::cout << std::endl;
    //std::cout << "A^(-1) " << A.inv() << std::endl;
    //std::cout << "A^(-1) * C " << A.inv() * C << std::endl;
    //std::cout << std::endl;
    cv::Mat At = A.t();
    std::cout << "(A^t * A)^(-1) * A^t * C " << (At * A).inv() * At * C << std::endl;
    std::cout << std::endl;

    //cv::Mat coef = A.inv() * C; // Specific form for 3 calibration points
    cv::Mat coef = (At * A).inv() * At * C; // Generic form for N calibration points
    float *coefData = (float*) coef.data;
    m_calibrationMatrix[pantilt] = QTransform(coefData[0], coefData[1],
            coefData[2], coefData[3],
            coefData[4], coefData[5]);
#else // Qt, shortcut, 3 or 4 points
    QPolygonF one;
    QPolygonF two;

    for (int i=0; i<4; ++i)
    {
        if (i < calibrationData.length())
        {
            one << calibrationData.at(i).first;
            two << calibrationData.at(i).second;
        }
        else
        {
            one << QPoint(0,0);
            two << QPoint(0,0);
        }
    }

    if (!QTransform::quadToQuad(one, two, m_calibrationMatrix[pantilt]))
    {
        qWarning() << "Could not calculate calibration matrix";
        return false;
    }
#endif

    m_calibrationMatrixInverted[pantilt] = m_calibrationMatrix[pantilt].inverted();

    qDebug() << "Calibration matrix:" << m_calibrationMatrix[pantilt];
    qDebug() << "Inverted calibration matrix:" << m_calibrationMatrixInverted[pantilt];

    m_parameterManager->setParameter(QString("pantilt/%1/calibration").arg(pantilt), m_calibrationMatrix[pantilt]);

    return true;
}

QTransform Hardware::calibrationMatrix(Pantilt pantilt)
{
    return m_calibrationMatrix[pantilt];
}

QPoint Hardware::screen2hardware(Pantilt pantilt, QPoint xyOnScreen) const
{
    return m_calibrationMatrixInverted[pantilt].map(xyOnScreen);
}

QPoint Hardware::hardware2screen(Pantilt pantilt, QPoint xyOnHardware) const
{
    return m_calibrationMatrix[pantilt].map(xyOnHardware);
}

Hardware::Speed Hardware::manualControlSpeed()
{
    return m_manualControlSpeed;
}

bool Hardware::setManualControlSpeed(Speed speed)
{
    m_manualControlSpeed = speed;
    return true;
}

bool Hardware::targetAbsolute(uint x, uint y, bool convertPos)
{
    bool ok = true;

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
        ok &= targetAbsolute(pantilt, x, y, convertPos);
    }

    return ok;
}

bool Hardware::targetRelative(qreal dx, qreal dy)
{
    bool ok = true;

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
        ok &= targetRelative(pantilt, dx, dy);
    }

    return ok;
}

bool Hardware::center()
{
    bool ok = true;

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
        ok &= center(pantilt);
    }

    return ok;
}

TehTrigger::TehTrigger(Hardware::Trigger trigger, Hardware *hardware) :
    QObject(hardware),
    m_trigger(trigger),
    m_hardware(hardware)
{
    m_firingTimer = new QTimer(this);
    m_firingTimer->setInterval(50);
    connect(m_firingTimer, SIGNAL(timeout()), SLOT(toggleFiring()));

    m_hwIsFiring = false;
    m_fireMode = Hardware::Pulse;
}

void TehTrigger::setFireMode(Hardware::FireMode fireMode)
{
    m_fireMode = fireMode;
}

bool TehTrigger::startFiring()
{
    bool ok = true;

    if (m_fireMode == Hardware::Pulse)
    {
        if (!m_hwIsFiring)
        {
            ok = m_hardware->hw_startFiring(m_trigger);
            if (ok) m_hwIsFiring = true;
        }
    }
    else if (m_fireMode == Hardware::FullyAutomatic ||
             m_fireMode == Hardware::Burst ||
             m_fireMode == Hardware::SingleShot)
    {
        m_burst = 0;

        if (!m_firingTimer->isActive())
        {
            m_firingTimer->start();
            toggleFiring();
        }
    }
    else if (m_fireMode == Hardware::Toggle)
    {
        toggleFiring();
    }

    return ok;
}

bool TehTrigger::stopFiring()
{
    bool ok = true;

    if (m_fireMode == Hardware::Pulse)
    {
        if (m_hwIsFiring)
        {
            ok = m_hardware->hw_stopFiring(m_trigger);
            if (ok) m_hwIsFiring = false;
        }
    }
    else if (m_fireMode == Hardware::FullyAutomatic)
    {
        if (m_firingTimer->isActive())
        {
            m_firingTimer->stop();

            ok = m_hardware->hw_stopFiring(m_trigger);
            if (ok) m_hwIsFiring = false;
        }
    }

    return ok;
}

void TehTrigger::toggleFiring()
{
    if (m_hwIsFiring)
        m_hwIsFiring = !m_hardware->hw_stopFiring(m_trigger);
    else
        m_hwIsFiring = m_hardware->hw_startFiring(m_trigger);

    if (m_fireMode == Hardware::SingleShot && !m_hwIsFiring)
    {
        m_firingTimer->stop();
    }
    else if (m_fireMode == Hardware::Burst && !m_hwIsFiring)
    {
        ++m_burst;
        if (m_burst >= 3)
        {
            m_firingTimer->stop();
        }
    }
}

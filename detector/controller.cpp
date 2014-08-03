#include "controller.h"
#include "audio.h"

#include "detector/movementdetector.h"
#include "detector/colordetector.h"
#include "detector/bodydetector.h"
#include "detector/facedetector.h"

Controller::Controller(QObject *parent) :
    QObject(parent),
    m_hardware(new Hardware()),
    m_audio(new Audio()),
    m_captureDevice(NULL),
    m_callibrating(false)
{
    m_detectors << new MovementDetector(this)
                << new ColorDetector(this)
                << new BodyDetector(this)
                << new FaceDetector(this);

    m_processTimer.setInterval(50);
    connect(&m_processTimer, SIGNAL(timeout()), SLOT(process()));
}

QList<Detector*> Controller::detectors()
{
    return m_detectors;
}

void Controller::setCaptureDevice(const QString &path)
{
    if (m_captureDevice)
    {
        m_captureDevice->release();
        delete m_captureDevice;
        m_captureDevice = NULL;
    }

    bool ok;
    int cam = path.toInt(&ok);
    if (ok)
    {
        m_captureDevice = new cv::VideoCapture(cam);
    }
    else
    {
        m_captureDevice = new cv::VideoCapture(path.toStdString());
    }
}

void Controller::startProcessing()
{
    m_processTimer.start();

    m_audio->playRandom(Audio::Deploy);
}

void Controller::stopProcessing()
{
    m_processTimer.stop();

    m_audio->playRandom(Audio::Retire);
}

void Controller::startCallibration(Hardware::Pantilt pantilt)
{
    m_currentPantiltCallibrating = pantilt;
    m_callibrating = true;
}

void Controller::nextCallibrationPoint()
{
    if (m_callibrationData.count() >= 3)
    {
        m_callibrating = false;
    }
}

void Controller::abortCallibration()
{
    m_callibrating = false;
}


bool Controller::targetAbsolute(Hardware::Pantilt pantilt, uint x, uint y)
{
    return m_hardware->targetAbsolute(pantilt, x, y);
}

bool Controller::targetRelative(Hardware::Pantilt pantilt, uint dx, uint dy)
{
    return m_hardware->targetRelative(pantilt, dx, dy);
}

bool Controller::enableFiring(Hardware::Gun gun)
{
    return m_hardware->enableFiring(gun);
}

bool Controller::stopFiring(Hardware::Gun gun)
{
    return m_hardware->stopFiring(gun);
}


void Controller::process()
{
    if (!m_captureDevice)
    {
        qCritical() << "No capture device";
        stopProcessing();
        return;
    }

    m_processTimer.stop();

    m_captureDevice->read(m_currentFrame);

    if (m_callibrating)
    {

    }
    else
    {
        foreach (Detector *detector, m_detectors)
        {
            if (detector->isEnabled())
            {
                QList<cv::Rect> detectedObjects = detector->detect(m_currentFrame);

                Q_UNUSED(detectedObjects);
            }
        }
    }

    emit newOpenCVFrame(m_currentFrame);

    m_processTimer.start();
}

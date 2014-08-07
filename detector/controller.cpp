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
    m_captureDevice(new cv::VideoCapture()),
    m_callibrating(false)
{
    m_detectors << new MovementDetector(this)
                << new ColorDetector(this)
                << new BodyDetector(this)
                << new FaceDetector(this);

    foreach (Detector *detector, m_detectors)
    {
        detector->init();
        m_objectColors.insert(detector, cv::Scalar(qrand()&255, qrand()&255, qrand()&255));
    }

    m_processTimer.setInterval(50);
    connect(&m_processTimer, SIGNAL(timeout()), SLOT(process()));
}

QList<Detector*> Controller::detectors()
{
    return m_detectors;
}

Audio *Controller::audio()
{
    return m_audio;
}

bool Controller::setCaptureDevice(int device)
{
    if (m_captureDevice->isOpened())
    {
        m_captureDevice->release();
    }

    return m_captureDevice->open(device);
}

bool Controller::setCaptureDevice(const QString &filename)
{
    if (m_captureDevice->isOpened())
    {
        m_captureDevice->release();
    }

    return m_captureDevice->open(filename.toStdString());
}

void Controller::startProcessing()
{
    m_processTimer.start();

    m_audio->play(Audio::Autosearch);
}

void Controller::stopProcessing()
{
    m_processTimer.stop();

    m_audio->play(Audio::Retire);
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
    if (!m_captureDevice->isOpened())
    {
        return;
    }

    m_processTimer.stop();

    if (!m_captureDevice->read(m_currentFrame))
    {
        qWarning() << "Could not capture frame";
        return;
    }

    if (m_callibrating)
    {
        // TODO
    }
    else
    {
        int targetRadius = 30;
        cv::Scalar potentialTargetColor = CV_RGB(255,255,255);
        cv::Scalar currentTargetColor = CV_RGB(255,0,0);

        foreach (Detector *detector, m_detectors)
        {
            if (detector->isEnabled())
            {
                QList<cv::Rect> detectedObjects = detector->detect(m_currentFrame);

                foreach (const cv::Rect &object, detectedObjects)
                {
                    cv::Scalar color = m_objectColors.value(detector);

                    cv::rectangle(m_currentFrame, object, color, 1, CV_AA, 0);

                    if (true) // TODO: Filter potential targets
                    {
                        cv::Point center(object.tl().x+object.width/2,
                                         object.tl().y+object.height/2);

                        cv::circle(m_currentFrame,
                                   center,
                                   targetRadius,
                                   potentialTargetColor,
                                   3, CV_AA, 0);

                        m_currentTarget = center; // TODO: Select current target
                    }
                }
            }
        }

        // Draw current target
        if (m_currentTarget.x>0 && m_currentTarget.y>0)
        {
            cv::circle(m_currentFrame,
                       m_currentTarget,
                       targetRadius,
                       currentTargetColor,
                       2, CV_AA, 0);

            cv::line(m_currentFrame,
                     cv::Point(m_currentTarget.x-targetRadius,
                               m_currentTarget.y),
                     cv::Point(m_currentTarget.x+targetRadius,
                               m_currentTarget.y),
                     currentTargetColor,
                     2, CV_AA, 0);

            cv::line(m_currentFrame,
                     cv::Point(m_currentTarget.x,
                               m_currentTarget.y-targetRadius),
                     cv::Point(m_currentTarget.x,
                               m_currentTarget.y+targetRadius),
                     currentTargetColor,
                     2, CV_AA, 0);
        }
    }

    emit newOpenCVFrame(m_currentFrame);

    m_processTimer.start();
}

int Controller::numCaptureDevices()
{
    int maxTested = 10;
    for (int i = 0; i < maxTested; i++)
    {
        cv::VideoCapture temp_camera(i);
        bool isOpened = temp_camera.isOpened();
        temp_camera.release();
        if (!isOpened)
        {
            return i;
        }
    }
    return maxTested;
}

#include "controller.h"
#include "audio.h"
#include "hardwareemulator.h"
#include "hardwarearduino.h"

#include "detector/movementdetector.h"
#include "detector/colordetector.h"
#include "detector/bodydetector.h"
#include "detector/facedetector.h"

Controller::Controller(QObject *parent) :
    QObject(parent),
    //m_hardware(new HardwareArduino()),
    m_hardware(new HardwareEmulator()),
    m_audio(new Audio()),
    m_captureDevice(new cv::VideoCapture()),
    m_videoWriter(new cv::VideoWriter()),
    m_callibrating(false),
    m_lastTargetId(0),
    m_currentTarget(NULL)
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


    m_hardware->currentPosition(Hardware::Body,
                                (uint&) m_currentBodyPosition.x,
                                (uint&) m_currentBodyPosition.y);
    m_hardware->currentPosition(Hardware::Eye,
                                (uint&) m_currentEyePosition.x,
                                (uint&) m_currentEyePosition.y);
    connect(m_hardware, &Hardware::currentPositionChanged,
            this, &Controller::onCurrentPositionChanged);
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

    if (m_captureDevice->isOpened())
    {
        m_captureDevice->release();
    }

    stopRecording();

    while (m_audio->isPlaying())
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

void Controller::startRecording()
{
    if (m_captureDevice->isOpened())
    {
        stopRecording();

        QString filename = "/tmp/test.avi";
        cv::Size inputSize = cv::Size((int) m_captureDevice->get(CV_CAP_PROP_FRAME_WIDTH),
                                      (int) m_captureDevice->get(CV_CAP_PROP_FRAME_HEIGHT));
        int fourcc = CV_FOURCC('M','P','E','G');
        double fps = m_captureDevice->get(CV_CAP_PROP_FPS);

        try
        {
            if (!m_videoWriter->open(filename.toStdString(), fourcc, fps, inputSize, true))
            {
                qWarning() << "Could not open video output" << filename;
            }
        }
        catch (cv::Exception e)
        {
            qCritical() << "Exception opening video output" << QString::fromStdString(e.msg);
        }
    }
}
void Controller::stopRecording()
{
    if (m_videoWriter->isOpened())
    {
        m_videoWriter->release();
    }
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
        QList<TrackingObject> trackingObjects;

        foreach (Detector *detector, m_detectors)
        {
            if (detector->isEnabled())
            {
                // Image --> Detected objects

                QList<cv::Rect> detectedObjects = detector->detect(m_currentFrame);

                foreach (const TrackingObject &object, m_trackingObjects)
                {
                    cv::rectangle(m_currentFrame,
                                  object.rect,
                                  m_objectColors.value(detector),
                                  1, CV_AA, 0);
                }

                // Detected objects --> Tracking objects
                // Tracking objects --> Potential targets

                trackingObjects << findTrackingObjects(detectedObjects,
                                                       m_trackingObjects,
                                                       detector);
            }
        }

        m_trackingObjects = trackingObjects;


        // Potential targets --> Current target

        m_currentTarget = findCurrentTarget(m_trackingObjects);
        if (m_currentTarget)
        {
            m_hardware->targetAbsolute(Hardware::Eye,
                                       m_currentTarget->center.x,
                                       m_currentTarget->center.y);
            m_hardware->targetAbsolute(Hardware::Body,
                                       m_currentTarget->center.x,
                                       m_currentTarget->center.y);
        }


        // Draw all tracking objects

        qulonglong currentTargetId = 0;
        if (m_currentTarget)
            currentTargetId = m_currentTarget->id;

        foreach (const TrackingObject &object, m_trackingObjects)
        {
            drawTrackingObject(m_currentFrame,
                               object,
                               object.id == currentTargetId);
        }


        // Draw hardware state

        drawCrosshair(m_currentFrame,
                      m_currentBodyPosition,
                      10,
                      CV_RGB(200,200,200),
                      1);

        drawCrosshair(m_currentFrame,
                      m_currentEyePosition,
                      5,
                      CV_RGB(240,10,10),
                      1);
    }

    emit newOpenCVFrame(m_currentFrame);

    if (m_videoWriter->isOpened())
    {
        m_videoWriter->write(m_currentFrame);
    }

    m_processTimer.start();
}

void Controller::onCurrentPositionChanged(Hardware::Pantilt pantilt, int x, int y)
{
    switch (pantilt)
    {
    case Hardware::Body:
        m_currentBodyPosition.x = x;
        m_currentBodyPosition.y = y;
        break;
    case Hardware::Eye:
        m_currentEyePosition.x = x;
        m_currentEyePosition.y = y;
        break;
    default:
        break;
    }
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

QList<Controller::TrackingObject> Controller::findTrackingObjects(const QList<cv::Rect> &detectedObjects,
                                                                  QList<TrackingObject> &lastFrameTrackingObjects,
                                                                  Detector *detector)
{
    QList<Controller::TrackingObject> targets;

    foreach (const cv::Rect &object, detectedObjects)
    {
        Controller::TrackingObject trackingObject;
        trackingObject.detector = detector;
        trackingObject.rect = object;
        trackingObject.center = cv::Point(object.tl().x+object.width/2,
                                          object.tl().y+object.height/2);


        trackingObject.id = 0;
        foreach (const TrackingObject &lastFrameTrackingObject, lastFrameTrackingObjects)
        {
            double distance = cv::norm(trackingObject.center - lastFrameTrackingObject.center);

            if (distance < 20)
            {
                trackingObject.id = lastFrameTrackingObject.id;
                //qDebug() << "Matched tracking object" << trackingObject.id;
                break; // TODO: Get the minimum distance, not the first that fits
            }
        }

        if (trackingObject.id == 0)
        {
            trackingObject.id = m_lastTargetId++;
            //qDebug() << "Creating new tracking object" << trackingObject.id;
        }


        // TODO Filter potential targets
        trackingObject.isTarget = true;


        targets << trackingObject;
    }

    return targets;
}

Controller::TrackingObject *Controller::findCurrentTarget(QList<Controller::TrackingObject> &trackingObjects)
{
    for (int i=0; i<trackingObjects.count(); ++i)
    {
        // TODO: Select current target
        return &trackingObjects[i];
    }

    return NULL;
}

void Controller::drawTrackingObject(cv::Mat &image,
                                    const TrackingObject &target,
                                    bool isCurrentTarget)
{
    cv::Scalar color;
    int thickness;
    int radius = 30;

    if (isCurrentTarget) // Red crosshair
    {
        color = CV_RGB(255,0,0);
        thickness = 2;

        drawCrosshair(image,
                      target.center,
                      radius,
                      color,
                      thickness);
    }
    else if (target.isTarget) // White crosshair
    {
        color = CV_RGB(255,255,255);
        thickness = 3;

        drawCrosshair(image,
                      target.center,
                      radius,
                      color,
                      thickness);
    }
    else // Circle
    {
        color = m_objectColors.value(target.detector);
        thickness = 1;

        cv::circle(image,
                   target.center,
                   radius,
                   color,
                   thickness,
                   CV_AA, 0);
    }

    cv::putText(m_currentFrame,
                QString("%1").arg(target.id).toStdString(),
                cv::Point(target.center.x-radius,
                          target.center.y+radius+15),
                cv::FONT_HERSHEY_PLAIN, 1,
                color,
                1, 8);
}

void Controller::drawCrosshair(cv::Mat &image,
                               const cv::Point &center,
                               int radius,
                               cv::Scalar color,
                               int thickness)
{
    cv::circle(image,
               center,
               radius,
               color,
               thickness,
               CV_AA, 0);

    cv::line(image,
             cv::Point(center.x-radius,
                       center.y),
             cv::Point(center.x+radius,
                       center.y),
             color,
             thickness,
             CV_AA, 0);

    cv::line(image,
             cv::Point(center.x,
                       center.y-radius),
             cv::Point(center.x,
                       center.y+radius),
             color,
             thickness,
             CV_AA, 0);
}

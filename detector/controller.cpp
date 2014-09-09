#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ocl/ocl.hpp>

#include "controller.h"
#include "audio.h"
#include "hardware/hardwareemulator.h"
#include "hardware/hardwarearduino.h"
#include "hardware/hardwarethunder.h"

#include "detector/movementdetector.h"
#include "detector/colordetector.h"
#include "detector/bodydetector.h"
#include "detector/facedetector.h"

#include "sentryinput.h"

Controller::Controller(QObject *parent) :
    QObject(parent),
    m_parameterManager(new ParameterManager(this, this)),
    m_processTimer(this),
    m_targetLostTimeout(this),
    m_sentryDisabledTimeout(this),
    m_sentryState(Retired),
    m_lastFpsMeasure(0),
    m_hardware(NULL),
    m_audio(new Audio()),
    m_captureDevice(new cv::VideoCapture()),
    m_videoWriter(new cv::VideoWriter()),
    m_trackingObjectId(0),
    m_processing(false),
    m_calibrating(false),
    m_currentTarget(NULL)
{
    try
    {
        qDebug() << "CUDA devices:" << cv::gpu::getCudaEnabledDeviceCount();
    }
    catch (cv::Exception e)
    {
        qWarning() << "Could not query CUDA devices:" << QString::fromStdString(e.msg);
    }

    try
    {
        cv::ocl::DevicesInfo oclDevices;
        qDebug() << "OpenCL devices:" << cv::ocl::getOpenCLDevices(oclDevices);
        foreach (const cv::ocl::DeviceInfo *device, oclDevices)
        {
            qDebug() << "  "
                     << device->deviceVendor.data()
                     << device->deviceName.data();
        }
    }
    catch (cv::Exception e)
    {
        qWarning() << "Could not query OpenCL devices:" << QString::fromStdString(e.msg);
    }

    connect(m_parameterManager, &ParameterManager::parametersChanged,
            this, &Controller::onParametersChanged);
    m_parameterManager->init();

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
    m_processTimer.start();

    m_targetLostTimeout.setInterval(1000);
    m_targetLostTimeout.setSingleShot(true);
    connect(&m_targetLostTimeout, SIGNAL(timeout()), SLOT(onTargetLost()));

    m_sentryDisabledTimeout.setInterval(3000);
    m_sentryDisabledTimeout.setSingleShot(true);
    connect(&m_sentryDisabledTimeout, SIGNAL(timeout()), SLOT(onDisabled()));

    m_hardware->currentPosition(Hardware::Body,
                                (uint&) m_currentPantiltPosition[Hardware::Body].x,
            (uint&) m_currentPantiltPosition[Hardware::Body].y);
    m_hardware->currentPosition(Hardware::Eye,
                                (uint&) m_currentPantiltPosition[Hardware::Eye].x,
            (uint&) m_currentPantiltPosition[Hardware::Eye].y);
    m_pantiltColor[Hardware::Body] = CV_RGB(200,200,200);
    m_pantiltColor[Hardware::Eye] = CV_RGB(240,10,10);
    m_pantiltRadius[Hardware::Body] = 10;
    m_pantiltRadius[Hardware::Eye] = 5;

    connect(m_hardware, &Hardware::currentPositionChanged,
            this, &Controller::onCurrentPositionChanged);
}

QString Controller::settingsGroup()
{
    return QString("Controller/");
}

ParameterList Controller::createParameters() const
{
    QVariantMap hardwareMap;
    hardwareMap.insert(tr("Arduino"), "arduino");
    hardwareMap.insert(tr("Thunder"), "thunder");
    hardwareMap.insert(tr("Emulator"), "emulator");

    ParameterList list;
    list << Parameter::selection("hardware", tr("Hardware"), hardwareMap, "arduino")
         << Parameter("aimTargets", tr("Aim targets"), Parameter::Boolean, true)
         << Parameter("tagTargets", tr("Tag targets"), Parameter::Boolean, true)
         << Parameter("shootTargets", tr("Shoot targets"), Parameter::Boolean, false)
         << Parameter("shootTolerance", tr("Shoot tolerance"), Parameter::Integer, 5, 0, 500);
    return list;
}

ParameterManager *Controller::parameterManager()
{
    return m_parameterManager;
}

QList<Detector*> Controller::detectors()
{
    return m_detectors;
}

Hardware *Controller::hardware()
{
    return m_hardware;
}

SentryInput *Controller::input()
{
    return m_input;
}

Audio *Controller::audio()
{
    return m_audio;
}

void Controller::setInput(SentryInput *input)
{
    m_input = input;
}

bool Controller::setCaptureDevice(int device)
{
    stopProcessing();

    if (m_captureDevice->isOpened())
    {
        m_captureDevice->release();
    }

    bool ok = m_captureDevice->open(device);

    if (ok)
    {
        startProcessing();
    }

    return ok;
}

bool Controller::setCaptureDevice(const QString &filename)
{
    stopProcessing();

    if (m_captureDevice->isOpened())
    {
        m_captureDevice->release();
    }

    bool ok = m_captureDevice->open(filename.toStdString());

    if (ok)
    {
        startProcessing();
    }

    return ok;
}

void Controller::startProcessing()
{
    m_processing = true;

    m_audio->play(Audio::Autosearch);
}

void Controller::stopProcessing()
{
    if (m_processing)
    {
        seSentrytState(Retired);
    }

    m_processing = false;

    m_hardware->stopFiring(Hardware::EyeLaser);

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

bool Controller::isCalibrating()
{
    return m_calibrating;
}

bool Controller::startCalibration()
{
    if (m_calibrating)
    {
        return false;
    }

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;

#if 0
        int minX, maxX, minY, maxY;
        if (!m_hardware->getLimits(pantilt, minX, maxX, minY, maxY))
        {
            return false;
        }

        m_calibrationData[pantilt].clear();
        m_calibrationHwPoints[pantilt] << QPoint(minX, minY)
                                       << QPoint(minX, maxY)
                                       << QPoint(maxX, maxY);
#else
        m_calibrationData[pantilt].clear();
        m_calibrationHwPoints[pantilt] << QPoint(90, 90)
                                       << QPoint(90-5, 90)
                                       << QPoint(90+5, 90+5);
#endif

        QPoint newHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
        qDebug() << "Pantilt" << pantilt << "First calibration point:" << newHardwarePoint;

        m_hardware->targetAbsolute(pantilt,
                                   newHardwarePoint.x(),
                                   newHardwarePoint.y(),
                                   false);
    }

    m_calibrating = true;

    return true;
}

bool Controller::nextCalibrationPoint(Hardware::Pantilt pantilt, QPoint screenPos)
{
    if (!m_calibrating)
    {
        return false;
    }

    int nCalibrationPoints = m_calibrationHwPoints[pantilt].count();

    if (m_calibrationData[pantilt].count() < nCalibrationPoints)
    {
        QPoint currentHardwarePoint;
        if (!m_hardware->currentPosition(pantilt,
                                         (uint&)currentHardwarePoint.rx(),
                                         (uint&)currentHardwarePoint.ry()))
        {
            qWarning() << "Could not get current position";
            return false;
        }

        qDebug() << "Pantilt" << pantilt << "Calibration pair added:" << currentHardwarePoint << screenPos;
        m_calibrationData[pantilt] << Hardware::PointPair(currentHardwarePoint, screenPos);

        if (m_calibrationData[pantilt].count() < nCalibrationPoints)
        {
            QPoint newHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
            qDebug() << "Pantilt" << pantilt << "New calibration point:" << newHardwarePoint;

            m_hardware->targetAbsolute(pantilt,
                                       newHardwarePoint.x(),
                                       newHardwarePoint.y(),
                                       false);
        }
        else
        {
            qDebug() << "Pantilt" << pantilt << "Calibration finished";

            if (!m_hardware->setCalibrationData(pantilt, m_calibrationData[pantilt]))
            {
                qWarning() << "Could not set calibration data";
            }
        }
    }

    if (m_calibrationData[Hardware::Body].count() >= nCalibrationPoints &&
            m_calibrationData[Hardware::Eye].count() >= nCalibrationPoints)
    {
        abortCallbration();
    }

    return m_calibrating;
}

bool Controller::abortCallbration()
{
    if (!m_calibrating)
    {
        return false;
    }

    m_calibrating = false;

    return true;
}


bool Controller::targetAbsolute(Hardware::Pantilt pantilt, uint x, uint y)
{
    return m_hardware->targetAbsolute(pantilt, x, y);
}

bool Controller::targetRelative(Hardware::Pantilt pantilt, qreal dx, qreal dy)
{
    return m_hardware->targetRelative(pantilt, dx, dy);
}

bool Controller::center(Hardware::Pantilt pantilt)
{
    return m_hardware->center(pantilt);
}

bool Controller::startFiring(Hardware::Trigger trigger)
{
    return m_hardware->startFiring(trigger);
}

bool Controller::stopFiring(Hardware::Trigger trigger)
{
    return m_hardware->stopFiring(trigger);
}


void Controller::process()
{
    m_processTimer.stop();

    ProfilingData profilingData;
    QTime profilingTimer;
    profilingTimer.start();

    if (!m_captureDevice->isOpened() || !m_captureDevice->read(m_currentFrame))
    {
        m_currentFrame = cv::Mat::zeros(480, 640, CV_8UC3);

        std::string text = tr("(No signal)").toStdString();
        cv::Size size = getTextSize(text,
                                    cv::FONT_HERSHEY_PLAIN, 1, 1, 0);
        cv::putText(m_currentFrame,
                    text,
                    cv::Point((m_currentFrame.cols - size.width) / 2,
                              (m_currentFrame.rows - size.height) / 2),
                    cv::FONT_HERSHEY_PLAIN, 1,
                    CV_RGB(255,255,255),
                    1, 8);
    }

    profilingData << ProfilingValue("readImage", profilingTimer.elapsed());

    if (m_processing && !m_calibrating)
    {
        int profilingTimerTracking = 0;

        QList<TrackingObject> trackingObjects;

        foreach (Detector *detector, m_detectors)
        {
            if (detector->isEnabled())
            {
                // Image --> Detected objects

                profilingTimer.start();

                QList<cv::Rect> detectedObjects = detector->detect(m_currentFrame);

                profilingData << ProfilingValue(detector->name(), profilingTimer.elapsed());

                foreach (const TrackingObject &object, m_trackingObjects)
                {
                    cv::rectangle(m_currentFrame,
                                  object.rect,
                                  m_objectColors.value(detector),
                                  1, CV_AA, 0);
                }

                // Detected objects --> Tracking objects
                // Tracking objects --> Potential targets

                profilingTimer.start();

                trackingObjects << findTrackingObjects(detectedObjects,
                                                       m_trackingObjects,
                                                       detector);

                profilingTimerTracking += profilingTimer.elapsed();
            }
        }

        profilingData << ProfilingValue("tracking", profilingTimerTracking);


        m_trackingObjects = trackingObjects;


        // Potential targets --> Current target

        profilingTimer.start();

        QString sentryStateStr;
        TrackingObject *previousTarget = m_currentTarget;
        m_currentTarget = findCurrentTarget(m_trackingObjects);

        profilingData << ProfilingValue("findCurrentTarget", profilingTimer.elapsed());

        if (m_currentTarget)
        {
            if (!previousTarget)
            {
                seSentrytState(Deployed);
            }

            if (m_parameterManager->value("tagTargets").toBool())
            {
                targetAbsolute(Hardware::Eye,
                               m_currentTarget->center.x,
                               m_currentTarget->center.y);
            }

            if (m_parameterManager->value("aimTargets").toBool())
            {
                targetAbsolute(Hardware::Body,
                               m_currentTarget->center.x,
                               m_currentTarget->center.y);
            }

            sentryStateStr = tr("Targeting: %1").arg(m_currentTarget->id);
        }
        else
        {
            if (previousTarget)
            {
                seSentrytState(Searching);
            }

            switch (m_sentryState)
            {
            case Disabled: sentryStateStr = tr("Disabled"); break;
            case Deployed: sentryStateStr = tr("Deployed"); break;
            case Searching: sentryStateStr = tr("Searching"); break;
            case Retired: sentryStateStr = tr("Retired"); break;
            }
        }

        cv::putText(m_currentFrame,
                    sentryStateStr.toStdString(),
                    cv::Point(2, m_currentFrame.rows-2),
                    cv::FONT_HERSHEY_PLAIN, 1,
                    CV_RGB(255,255,255),
                    1, 8);


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
    }
    else
    {
        m_trackingObjects.clear();
    }


    // Draw hardware state

    for (int i=Hardware::Body; i<Hardware::__Pantilt_Size__; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
        drawCrosshair(m_currentFrame,
                      m_currentPantiltPosition[pantilt],
                      m_pantiltRadius[pantilt],
                      m_pantiltColor[pantilt],
                      1);
    }


    // Count FPS
    if (m_fpsTimer.isValid())
    {
        double fps = 1000.0 / m_fpsTimer.elapsed();
        m_lastFpsMeasure = (fps + m_lastFpsMeasure)/2;

        std::string str = QString("FPS: %1").arg(m_lastFpsMeasure).toStdString();
        cv::Size size = cv::getTextSize(str, cv::FONT_HERSHEY_PLAIN, 1, 1, 0);
        cv::putText(m_currentFrame,
                    str,
                    cv::Point(2, 2+size.height),
                    cv::FONT_HERSHEY_PLAIN, 1,
                    CV_RGB(255,255,255),
                    1, 8);
    }
    m_fpsTimer.start();


    emit newOpenCVFrame(m_currentFrame);
    emit newProfilingData(profilingData);


    if (m_videoWriter->isOpened())
    {
        m_videoWriter->write(m_currentFrame);
    }

    m_processTimer.start();
}

void Controller::onDeployed()
{
    m_targetLostTimeout.stop();
    m_sentryDisabledTimeout.stop();

    if (m_parameterManager->value("tagTargets").toBool())
    {
        m_hardware->startFiring(Hardware::EyeLaser);
    }

    m_audio->play(Audio::Deploy);
}

void Controller::onTargetLost()
{
    seSentrytState(Searching);

    if (m_parameterManager->value("tagTargets").toBool())
    {
        m_hardware->stopFiring(Hardware::EyeLaser);
    }

    m_audio->play(Audio::Search);

    m_hardware->center();

    m_sentryDisabledTimeout.start();
}

void Controller::onDisabled()
{
    seSentrytState(Disabled);

    m_audio->play(Audio::Disabled);
}

void Controller::onRetired()
{
    m_audio->play(Audio::Retire);
}

void Controller::onParametersChanged()
{
    if (m_hardware)
    {
        delete m_hardware;
        m_hardware = NULL;
    }

    QString hardware = m_parameterManager->value("hardware").toString();
    if (hardware == "arduino")
        m_hardware = new HardwareArduino();
    else if (hardware == "thunder")
        m_hardware = new HardwareThunder();
    else if (hardware == "emulator")
        m_hardware = new HardwareEmulator();

    if (m_hardware)
    {
        qDebug() << "Initializing hardware" << hardware;
        m_hardware->init();
    }
    else
    {
        qCritical() << "Invalid hardware name" << hardware;
    }
}

void Controller::onCurrentPositionChanged(Hardware::Pantilt pantilt, int x, int y)
{
    m_currentPantiltPosition[pantilt].x = x;
    m_currentPantiltPosition[pantilt].y = y;

    if (m_currentTarget
            && pantilt == Hardware::Body
            && m_parameterManager->value("shootTargets").toBool())
    {
        double distance = cv::norm(m_currentPantiltPosition[pantilt] - m_currentTarget->center);

        if (distance < m_parameterManager->value("shootTolerance").toInt())
        {
            m_hardware->startFiring(Hardware::LeftGun);
        }
        else
        {
            m_hardware->stopFiring(Hardware::LeftGun);
        }
    }
    else
    {
        m_hardware->stopFiring(Hardware::LeftGun);
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
            trackingObject.id = m_trackingObjectId++;
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
    cv::Point realCenter;
    realCenter.x = qMax(0, qMin(center.x, image.cols));
    realCenter.y = qMax(0, qMin(center.y, image.rows));

    if (center != realCenter)
    {
        cv::putText(m_currentFrame,
                    QString("(%1,%2)").arg(center.x).arg(center.y).toStdString(),
                    cv::Point(realCenter.x,
                              realCenter.y + 20),
                    cv::FONT_HERSHEY_PLAIN, 1,
                    color,
                    1, 8);
    }

    cv::circle(image,
               realCenter,
               radius,
               color,
               thickness,
               CV_AA, 0);

    cv::line(image,
             cv::Point(realCenter.x-radius,
                       realCenter.y),
             cv::Point(realCenter.x+radius,
                       realCenter.y),
             color,
             thickness,
             CV_AA, 0);

    cv::line(image,
             cv::Point(realCenter.x,
                       realCenter.y-radius),
             cv::Point(realCenter.x,
                       realCenter.y+radius),
             color,
             thickness,
             CV_AA, 0);
}

void Controller::seSentrytState(State state)
{
    if (m_sentryState != state)
    {
        switch (state)
        {
        case Disabled:
            break;

        case Deployed:
            onDeployed();
            break;

        case Searching:
            m_targetLostTimeout.start();
            break;

        case Retired:
            onRetired();
            break;
        }

        m_sentryState = state;
    }
}

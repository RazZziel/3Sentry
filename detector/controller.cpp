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

Controller::Controller(QObject *parent) :
    QObject(parent),
    m_parameterManager(new ParameterManager(this, this)),
    m_processTimer(this),
    m_hardware(NULL),
    m_audio(new Audio()),
    m_captureDevice(new cv::VideoCapture()),
    m_videoWriter(new cv::VideoWriter()),
    m_trackingObjectId(0),
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

    m_parameterManager->init();

    //m_hardware = new HardwareArduino();
    m_hardware = new HardwareEmulator();
    //m_hardware = new HardwareThunder();
    m_hardware->init();

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
    ParameterList list;
    list << Parameter("foo", tr("Le foo"), Parameter::String, "bar");
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

Audio *Controller::audio()
{
    return m_audio;
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
    m_processTimer.start();

    m_audio->play(Audio::Autosearch);
}

void Controller::stopProcessing()
{
    if (m_processTimer.isActive())
    {
        m_audio->play(Audio::Retire);
    }

    m_processTimer.stop();

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

    for (int i=Hardware::Body; i<=Hardware::Eye; i++)
    {
        Hardware::Pantilt pantilt = (Hardware::Pantilt) i;

        int minX, maxX, minY, maxY;
        if (!m_hardware->getLimits(pantilt, minX, maxX, minY, maxY))
        {
            return false;
        }

        m_calibrationData[pantilt].clear();
        m_calibrationHwPoints[pantilt] << QPoint(minX, minY)
                                       << QPoint(minX, maxY)
                                       << QPoint(maxX, maxY);

        QPoint newHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
        qDebug() << "Pantilt" << pantilt << "First calibration point:" << newHardwarePoint;
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
        QPoint currentHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
        qDebug() << "Pantilt" << pantilt << "Calibration pair added:" << currentHardwarePoint << screenPos;
        m_calibrationData[pantilt] << Hardware::PointPair(currentHardwarePoint, screenPos);

        if (m_calibrationData[pantilt].count() < nCalibrationPoints)
        {
            QPoint newHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
            qDebug() << "Pantilt" << pantilt << "New calibration point:" << newHardwarePoint;
        }
        else
        {
            qDebug() << "Pantilt" << pantilt << "Calibration finished";
        }
    }
    else if (m_calibrationData[pantilt].count() == nCalibrationPoints)
    {
        if (!m_hardware->setCalibrationData(pantilt, m_calibrationData[pantilt]))
        {
            qWarning() << "Could not set calibration data";
        }
    }

    if (m_calibrationData[Hardware::Body].count() >= nCalibrationPoints &&
            m_calibrationData[Hardware::Eye].count() >= nCalibrationPoints)
    {
        m_calibrating = false;
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

    if (m_calibrating)
    {
        for (int i=Hardware::Body; i<=Hardware::Eye; i++)
        {
            Hardware::Pantilt pantilt = (Hardware::Pantilt) i;

            if (m_calibrationData[pantilt].count() < m_calibrationHwPoints[pantilt].count())
            {
                QPoint currentHardwarePoint = m_calibrationHwPoints[pantilt][m_calibrationData[pantilt].count()];
                m_hardware->targetAbsolute(pantilt,
                                           currentHardwarePoint.x(),
                                           currentHardwarePoint.y(),
                                           false);


                drawCrosshair(m_currentFrame,
                              m_currentPantiltPosition[pantilt],
                              m_pantiltRadius[pantilt],
                              m_pantiltColor[pantilt],
                              1);

                cv::putText(m_currentFrame,
                            QString("(%1,%2)")
                            .arg(currentHardwarePoint.x())
                            .arg(currentHardwarePoint.y())
                            .toStdString(),
                            cv::Point(m_currentPantiltPosition[pantilt].x,
                                      m_currentPantiltPosition[pantilt].y + m_pantiltRadius[pantilt] + 10),
                            cv::FONT_HERSHEY_PLAIN, 1,
                            m_pantiltColor[pantilt],
                            1, 8);
            }
        }
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
            targetAbsolute(Hardware::Eye,
                           m_currentTarget->center.x,
                           m_currentTarget->center.y);
            targetAbsolute(Hardware::Body,
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

        for (int i=Hardware::Body; i<=Hardware::Eye; i++)
        {
            Hardware::Pantilt pantilt = (Hardware::Pantilt) i;
            drawCrosshair(m_currentFrame,
                          m_currentPantiltPosition[pantilt],
                          m_pantiltRadius[pantilt],
                          m_pantiltColor[pantilt],
                          1);
        }
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
    m_currentPantiltPosition[pantilt].x = x;
    m_currentPantiltPosition[pantilt].y = y;
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

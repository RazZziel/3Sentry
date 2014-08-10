#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <cv.h>
#include <highgui.h>
#include "hardware/hardware.h"

class Detector;
class Hardware;
class Audio;

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent=0);

    QList<Detector*> detectors();
    Audio *audio();

    int numCaptureDevices();
    bool setCaptureDevice(int device);
    bool setCaptureDevice(const QString &filename);

public slots:
    void startProcessing();
    void stopProcessing();

    void startRecording();
    void stopRecording();

    bool isCalibrating();
    bool startCallibration();
    bool nextCallibrationPoint(Hardware::Pantilt pantilt, QPoint screenPos);
    bool abortCallibration();

    bool targetAbsolute(Hardware::Pantilt pantilt, uint x, uint y);
    bool targetRelative(Hardware::Pantilt pantilt, uint dx, uint dy);
    bool enableFiring(Hardware::Gun gun);
    bool stopFiring(Hardware::Gun gun);

private:
    struct TrackingObject {
        qulonglong id;
        cv::Rect rect;
        cv::Point center;
        bool isTarget;
        Detector *detector;
    };

    QList<TrackingObject> findTrackingObjects(const QList<cv::Rect> &detectedObjects,
                                              QList<TrackingObject> &lastFrameTrackingObjects,
                                              Detector *detector);
    TrackingObject *findCurrentTarget(QList<TrackingObject> &trackingObjects);

    void drawTrackingObject(cv::Mat &image,
                            const TrackingObject &target,
                            bool isCurrentTarget);
    void drawCrosshair(cv::Mat &image,
                       const cv::Point &center,
                       int radius,
                       cv::Scalar color,
                       int thickness);

    QTimer m_processTimer;

    QList<Detector*> m_detectors;
    Hardware *m_hardware;
    Audio *m_audio;

    cv::VideoCapture* m_captureDevice;
    cv::VideoWriter* m_videoWriter;
    cv::Mat m_currentFrame;

    qulonglong m_trackingObjectId;

    bool m_calibrating;
    QHash<Hardware::Pantilt,QPoint> m_calibrationCurrentHwPos;
    QHash<Hardware::Pantilt,QList<QPoint> > m_calibrationHwPoints;
    QHash<Hardware::Pantilt,Hardware::CalibrationData> m_calibrationData;

    QMap<Detector*, cv::Scalar> m_objectColors;
    QList<TrackingObject> m_trackingObjects;
    TrackingObject *m_currentTarget;

    QHash<Hardware::Pantilt,int> m_pantiltRadius;
    QHash<Hardware::Pantilt,cv::Scalar> m_pantiltColor;
    QHash<Hardware::Pantilt,cv::Point> m_currentPantiltPosition;

private slots:
    void process();
    void onCurrentPositionChanged(Hardware::Pantilt pantilt, int x, int y);

signals:
    void newOpenCVFrame(cv::Mat image);
};

#endif // CONTROLLER_H

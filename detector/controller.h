#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <cv.h>
#include <highgui.h>
#include "hardware/hardware.h"
#include "parametermanager.h"

class Detector;
class Hardware;
class Audio;

class Controller : public QObject, public ParameterOwner
{
    Q_OBJECT
public:
    Controller(QObject *parent=0);

    QList<Detector*> detectors();
    Hardware *hardware();
    Audio *audio();

    int numCaptureDevices();
    bool setCaptureDevice(int device);
    bool setCaptureDevice(const QString &filename);

    QString settingsGroup();
    ParameterList createParameters() const;
    ParameterManager *parameterManager();

public slots:
    void startProcessing();
    void stopProcessing();

    void startRecording();
    void stopRecording();

    bool isCalibrating();
    bool startCalibration();
    bool nextCalibrationPoint(Hardware::Pantilt pantilt, QPoint screenPos);
    bool abortCallbration();

    bool targetAbsolute(Hardware::Pantilt pantilt, uint x, uint y);
    bool targetRelative(Hardware::Pantilt pantilt, qreal dx, qreal dy);
    bool startFiring(Hardware::Trigger trigger);
    bool stopFiring(Hardware::Trigger trigger);

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


    ParameterManager *m_parameterManager;
    QTimer m_processTimer;
    QTimer m_targetMissTimeout;

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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <cv.h>
#include <highgui.h>
#include "hardware.h"

class Detector;
class Hardware;
class Audio;

class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent=0);

    QList<Detector*> detectors();

    void setCaptureDevice(const QString &path);

    void startProcessing();
    void stopProcessing();

    void startCallibration(Hardware::Pantilt pantilt);
    void nextCallibrationPoint();
    void abortCallibration();

private:
    QTimer m_processTimer;

    QList<Detector*> m_detectors;
    Hardware *m_hardware;
    Audio *m_audio;

    cv::VideoCapture* m_captureDevice;
    cv::Mat m_currentFrame;

    bool m_callibrating;
    Hardware::Pantilt m_currentPantiltCallibrating;
    Hardware::CalibrationData m_callibrationData;

private slots:
    void process();

signals:
    void newOpenCVFrame(cv::Mat image);
};

#endif // CONTROLLER_H

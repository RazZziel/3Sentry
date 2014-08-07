#ifndef CASCADECLASSIFIERDETECTOR_H
#define CASCADECLASSIFIERDETECTOR_H

#include "detector.h"

#define OPENCV_CASCADE_DIR "/usr/share/opencv/haarcascades/"

class CascadeClassifierDetector : public Detector
{
    Q_OBJECT
public:
    CascadeClassifierDetector(const QString &filename, QObject *parent=0);

    QString name();
    QList<cv::Rect> detect(const cv::Mat& image) const;

private:
    DetectorParameterList createParameters() const;

    cv::CascadeClassifier* m_classifier;
};

#endif // CASCADECLASSIFIERDETECTOR_H

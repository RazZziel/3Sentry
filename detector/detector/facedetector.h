#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include "detector.h"

class FaceDetector : public Detector
{
    Q_OBJECT
public:
    FaceDetector(QObject *parent=0);

    QString name();
    QList<cv::Rect> detect(const cv::Mat& image) const;

private:
    cv::CascadeClassifier* m_classifier;
};

#endif // FACEDETECTOR_H

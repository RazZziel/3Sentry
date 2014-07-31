#include "facedetector.h"

FaceDetector::FaceDetector(QObject *parent) :
    Detector(parent)
{
}

QString FaceDetector::name()
{
    return tr("Face detector");
}

QList<cv::Rect> FaceDetector::detect(const cv::Mat& image) const
{
    Q_UNUSED(image);
    return QList<cv::Rect>();
}

#include "bodydetector.h"

BodyDetector::BodyDetector(QObject *parent) :
    Detector(parent)
{
}

QString BodyDetector::name()
{
    return tr("Body detector");
}

QList<cv::Rect> BodyDetector::detect(const cv::Mat& image) const
{
    Q_UNUSED(image);
    return QList<cv::Rect>();
}

#include "colordetector.h"

ColorDetector::ColorDetector(QObject *parent) :
    Detector(parent)
{
}

QString ColorDetector::name()
{
    return tr("Color detector");
}

QList<cv::Rect> ColorDetector::detect(const cv::Mat& image) const
{
    Q_UNUSED(image);
    return QList<cv::Rect>();
}

#include "movementdetector.h"

MovementDetector::MovementDetector(QObject *parent) :
    Detector(parent)
{
}

QString MovementDetector::name()
{
    return tr("Movement detector");
}

QList<cv::Rect> MovementDetector::detect(const cv::Mat& image) const
{
    Q_UNUSED(image);
    return QList<cv::Rect>();
}

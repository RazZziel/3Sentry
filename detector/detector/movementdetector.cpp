#include "movementdetector.h"

MovementDetector::MovementDetector(QObject *parent) :
    Detector(parent)
{
}

QString MovementDetector::name() const
{
    return tr("Movement detector");
}

QList<cv::Rect> MovementDetector::detect(const cv::Mat& image) const
{
    // TODO: https://www.youtube.com/watch?v=X6rPdRZzgjg

    Q_UNUSED(image);
    return QList<cv::Rect>();
}

ParameterList MovementDetector::createParameters() const
{
    ParameterList list = Detector::createParameters();
    return list;
}

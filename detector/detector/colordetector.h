#ifndef COLORDETECTOR_H
#define COLORDETECTOR_H

#include "detector.h"

class ColorDetector : public Detector
{
    Q_OBJECT
public:
    ColorDetector(QObject *parent=0);

    QString name();
    QList<cv::Rect> detect(const cv::Mat& image) const;

private:
    DetectorParameterList createParameters() const;
};

#endif // COLORDETECTOR_H

#ifndef MOVEMENTDETECTOR_H
#define MOVEMENTDETECTOR_H

#include "detector.h"

class MovementDetector : public Detector
{
    Q_OBJECT
public:
    MovementDetector(QObject *parent=0);

    QString name() const;
    QList<cv::Rect> detect(const cv::Mat& image) const;

private:
    ParameterList createParameters() const;
};

#endif // MOVEMENTDETECTOR_H

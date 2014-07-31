#ifndef MOVEMENTDETECTOR_H
#define MOVEMENTDETECTOR_H

#include "detector.h"

class MovementDetector : public Detector
{
    Q_OBJECT
public:
    MovementDetector(QObject *parent=0);

    QString name();
    QList<cv::Rect> detect(const cv::Mat& image) const;
};

#endif // MOVEMENTDETECTOR_H

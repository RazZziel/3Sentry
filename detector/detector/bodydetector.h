#ifndef BODYDETECTOR_H
#define BODYDETECTOR_H

#include "detector.h"

class BodyDetector : public Detector
{
    Q_OBJECT
public:
    BodyDetector(QObject *parent=0);

    QString name();
    QList<cv::Rect> detect(const cv::Mat& image) const;
};

#endif // BODYDETECTOR_H

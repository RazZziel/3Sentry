#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include "cascadeclassifierdetector.h"

class FaceDetector : public CascadeClassifierDetector
{
    Q_OBJECT
public:
    FaceDetector(QObject *parent=0);

    QString name() const;
};

#endif // FACEDETECTOR_H

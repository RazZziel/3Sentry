#ifndef BODYDETECTOR_H
#define BODYDETECTOR_H

#include "cascadeclassifierdetector.h"

class BodyDetector : public CascadeClassifierDetector
{
    Q_OBJECT
public:
    BodyDetector(QObject *parent=0);

    QString name();
};

#endif // BODYDETECTOR_H

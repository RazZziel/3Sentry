#include "bodydetector.h"

BodyDetector::BodyDetector(QObject *parent) :
    CascadeClassifierDetector(OPENCV_CASCADE_DIR "haarcascade_fullbody.xml", parent)
{
}

QString BodyDetector::name() const
{
    return tr("Body detector");
}

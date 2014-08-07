#include "facedetector.h"

FaceDetector::FaceDetector(QObject *parent) :
    CascadeClassifierDetector(OPENCV_CASCADE_DIR "haarcascade_frontalface_default.xml", parent)
{
}

QString FaceDetector::name()
{
    return tr("Face detector");
}

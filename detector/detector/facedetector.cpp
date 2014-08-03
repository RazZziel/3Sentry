#include "facedetector.h"

#define OPENCV_DATA "/usr/share/opencv/haarcascades/"

FaceDetector::FaceDetector(QObject *parent) :
    Detector(parent),
    m_classifier(new cv::CascadeClassifier())
{
    m_classifier->load(OPENCV_DATA "haarcascade_frontalface_default.xml");

    m_enabled = true; // TESTING
}

QString FaceDetector::name()
{
    return tr("Face detector");
}

QList<cv::Rect> FaceDetector::detect(const cv::Mat& image) const
{
    std::vector<cv::Rect> objects;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;

    m_classifier->detectMultiScale(image,
                                   objects,
                                   rejectLevels,
                                   levelWeights,
                                   1.1,                       //increase search scale by 10% each pass
                                   6,                         //require 6 neighbors
                                   CV_HAAR_DO_CANNY_PRUNING,  //skip regions unlikely to contain a face
                                   cv::Size(100, 100),            //use default face size from xml
                                   cv::Size(500, 500));

    QList<cv::Rect> objectList;
    for (uint i=0; i<objects.size(); ++i)
    {
        objectList << objects.at(i);
    }

    return objectList;
}

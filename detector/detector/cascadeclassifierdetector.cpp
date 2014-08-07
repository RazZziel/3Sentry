#include "cascadeclassifierdetector.h"

CascadeClassifierDetector::CascadeClassifierDetector(const QString &filename, QObject *parent) :
    Detector(parent),
    m_classifier(new cv::CascadeClassifier())
{
    m_classifier->load(filename.toStdString());

#ifdef QT_DEBUG // TESTING
    m_enabled = true;
#endif
}

QString CascadeClassifierDetector::name()
{
    return tr("Cascade classifier detector");
}

QList<cv::Rect> CascadeClassifierDetector::detect(const cv::Mat& image) const
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

DetectorParameterList CascadeClassifierDetector::createParameters() const
{
    DetectorParameterList list = Detector::createParameters();
    list << DetectorParameter("limitWidth", tr("Limit width"), DetectorParameter::Boolean, true)
         << DetectorParameter("minWidth", tr("Minimum width"), DetectorParameter::Integer, 100, 0, 1000)
         << DetectorParameter("maxWidth", tr("Maximum width"), DetectorParameter::Integer, 500, 0, 1000)

         << DetectorParameter("limitHeight", tr("Limit height"), DetectorParameter::Boolean, true)
         << DetectorParameter("minHeight", tr("Minimum height"), DetectorParameter::Integer, 100, 0, 1000)
         << DetectorParameter("maxHeight", tr("Maximum height"), DetectorParameter::Integer, 500, 0, 1000)

         << DetectorParameter("limitArea", tr("Limit area"), DetectorParameter::Boolean, false)
         << DetectorParameter("minArea", tr("Minimum height"), DetectorParameter::Integer, 10000, 0, 1000)
         << DetectorParameter("maxArea", tr("Maximum height"), DetectorParameter::Integer, 250000, 0, 1000)

         << DetectorParameter("scaleFactor", tr("Scale factor"), DetectorParameter::Real, 1.1, 0, 1000)
         << DetectorParameter("minNeighboors", tr("Maximum neighboors"), DetectorParameter::Integer, 6, 0, 10);
    return list;
}

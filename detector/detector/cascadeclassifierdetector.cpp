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

QString CascadeClassifierDetector::name() const
{
    return tr("Cascade classifier detector");
}

QList<cv::Rect> CascadeClassifierDetector::detect(const cv::Mat& image) const
{
    std::vector<cv::Rect> objects;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;

    double scaleFactor = m_parameters.value("scaleFactor").m_value.toDouble();
    int minNeighbors = m_parameters.value("minNeighbors").m_value.toInt();

    int minWidth = m_parameters.value("minWidth").m_value.toInt();
    int minHeight = m_parameters.value("minHeight").m_value.toInt();
    int maxWidth = m_parameters.value("maxWidth").m_value.toInt();
    int maxHeight = m_parameters.value("maxHeight").m_value.toInt();

    try
    {
        m_classifier->detectMultiScale(image,
                                       objects,
                                       rejectLevels,
                                       levelWeights,
                                       scaleFactor,
                                       minNeighbors,
                                       CV_HAAR_DO_CANNY_PRUNING,  // skip regions unlikely to contain an object
                                       cv::Size(minWidth, minHeight),
                                       cv::Size(maxWidth, maxHeight));
    }
    catch (cv::Exception e)
    {
        qWarning() << name() << "failed:" << QString::fromStdString(e.msg);
    }

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
    list << DetectorParameter("scaleFactor", tr("Scale factor"), DetectorParameter::Real, 1.1, 0, 1000)
         << DetectorParameter("minNeighbors", tr("Maximum neighboors"), DetectorParameter::Integer, 6, 0, 10);
    return list;
}

QList<cv::Rect> CascadeClassifierDetector::filterResults(const QList<cv::Rect> &objects) const
{
    // Height and width are already trimmed down by the cascade classifier

    bool limitArea = m_parameters.value("limitArea").m_value.toBool();
    int minArea = m_parameters.value("minArea").m_value.toInt();
    int maxArea = m_parameters.value("maxArea").m_value.toInt();

    QList<cv::Rect> list;

    foreach (const cv::Rect &object, objects)
    {
        if (limitArea)
        {
            int area = object.area();
            if (area < minArea || area > maxArea)
            {
                continue;
            }
        }

        list << object;
    }

    return list;
}

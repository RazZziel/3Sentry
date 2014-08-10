#include <opencv2/gpu/gpu.hpp>
#include <opencv2/ocl/ocl.hpp>
#include "cascadeclassifierdetector.h"

CascadeClassifierDetector::CascadeClassifierDetector(const QString &filename, QObject *parent) :
    Detector(parent),
    m_classifier(new cv::CascadeClassifier()),
    m_classifier_CUDA(NULL),
    m_classifier_OCL(NULL)
{
    try
    {
        if (cv::gpu::getCudaEnabledDeviceCount() > 0)
        {
            m_classifier_CUDA = new cv::gpu::CascadeClassifier_GPU();
            m_classifier_CUDA->load(filename.toStdString());
            qDebug() << "Using CUDA classifier";
        }
    }
    catch (cv::Exception e)
    {
        qWarning() << name() << "Could not init CUDA classifier:" << QString::fromStdString(e.msg);
        m_classifier_CUDA = NULL;
    }

    try
    {
        cv::ocl::DevicesInfo oclDevices;
        if (cv::ocl::getOpenCLDevices(oclDevices) > 0)
        {
            m_classifier_OCL = new cv::ocl::OclCascadeClassifier();
            m_classifier_OCL->load(filename.toStdString());
            qDebug() << "Using OpenCL classifier";
        }
    }
    catch (cv::Exception e)
    {
        qWarning() << name() << "Could not init OpenCL classifier:" << QString::fromStdString(e.msg);
        m_classifier_OCL = NULL;
    }

    m_classifier->load(filename.toStdString());
}

QString CascadeClassifierDetector::name() const
{
    return tr("Cascade classifier detector");
}

QList<cv::Rect> CascadeClassifierDetector::detect(const cv::Mat& image) const
{
    bool useCudaIfAvailable = m_parameters.value("useCudaIfAvailable").m_value.toDouble();
    bool useOclIfAvailable = m_parameters.value("useOclIfAvailable").m_value.toBool();

    double scaleFactor = m_parameters.value("scaleFactor").m_value.toDouble();
    int minNeighbors = m_parameters.value("minNeighbors").m_value.toInt();

    cv::Size minSize(m_parameters.value("minWidth").m_value.toInt(),
                     m_parameters.value("minHeight").m_value.toInt());
    cv::Size maxSize(m_parameters.value("maxWidth").m_value.toInt(),
                     m_parameters.value("maxHeight").m_value.toInt());

    if (m_classifier_CUDA && useCudaIfAvailable)
    {
        return detectMultiScale_CUDA(image, minSize, maxSize, scaleFactor, minNeighbors);
    }
    else if (m_classifier_OCL && useOclIfAvailable)
    {
        return detectMultiScale_OCL(image, minSize, maxSize, scaleFactor, minNeighbors);
    }
    else
    {
        return detectMultiScale(image, minSize, maxSize, scaleFactor, minNeighbors);
    }
}

QList<cv::Rect> CascadeClassifierDetector::detectMultiScale(const cv::Mat& image,
                                                            cv::Size minSize, cv::Size maxSize,
                                                            double scaleFactor, int minNeighbors) const
{
    std::vector<cv::Rect> objects;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;

    try
    {
        m_classifier->detectMultiScale(image,
                                       objects,
                                       rejectLevels,
                                       levelWeights,
                                       scaleFactor,
                                       minNeighbors,
                                       CV_HAAR_DO_CANNY_PRUNING,  // skip regions unlikely to contain an object
                                       minSize,
                                       maxSize);
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

QList<cv::Rect> CascadeClassifierDetector::detectMultiScale_CUDA(const cv::Mat& image,
                                                                 cv::Size minSize, cv::Size maxSize,
                                                                 double scaleFactor, int minNeighbors) const
{
    cv::Mat objects_mat;
    int nObjects = 0;

    try
    {
        cv::gpu::GpuMat image_gpu;
        cv::gpu::GpuMat objects_gpu;

        image_gpu.upload(image);

        nObjects = m_classifier_CUDA->detectMultiScale(image_gpu, objects_gpu,
                                                       minSize, maxSize,
                                                       scaleFactor, minNeighbors);

        objects_gpu.colRange(0, nObjects).download(objects_mat);
    }
    catch (cv::Exception e)
    {
        qWarning() << name() << "failed:" << QString::fromStdString(e.msg);
        qWarning() << "Disabling CUDA...";
        delete m_classifier_CUDA;
        m_classifier_CUDA = NULL;
    }

    QList<cv::Rect> objectList;
    cv::Rect* faces = objects_mat.ptr<cv::Rect>();
    for (int i=0; i < nObjects; ++i)
    {
        objectList << faces[i];
    }

    return objectList;
}

QList<cv::Rect> CascadeClassifierDetector::detectMultiScale_OCL(const cv::Mat& image,
                                                                cv::Size minSize, cv::Size maxSize,
                                                                double scaleFactor, int minNeighbors) const
{
    std::vector<cv::Rect> objects;

    try
    {
        cv::ocl::oclMat image_gpu;
        image_gpu.upload(image);

        m_classifier_OCL->detectMultiScale(image_gpu, objects,
                                           scaleFactor, minNeighbors,
                                           0,
                                           minSize, maxSize);
    }
    catch (cv::Exception e)
    {
        qWarning() << name() << "failed:" << QString::fromStdString(e.msg);
        qWarning() << "Disabling OpenCL...";
        delete m_classifier_OCL;
        m_classifier_OCL = NULL;
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
         << DetectorParameter("minNeighbors", tr("Maximum neighboors"), DetectorParameter::Integer, 6, 0, 10)
         << DetectorParameter("useCudaIfAvailable", tr("Use CUDA if available"), DetectorParameter::Boolean, true)
         << DetectorParameter("useOclIfAvailable", tr("Use OpenCL if available"), DetectorParameter::Boolean, true);
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

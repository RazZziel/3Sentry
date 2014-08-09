#ifndef CASCADECLASSIFIERDETECTOR_H
#define CASCADECLASSIFIERDETECTOR_H

#include "detector.h"

#define OPENCV_CASCADE_DIR "/usr/share/opencv/haarcascades/"

namespace cv {
    namespace gpu {
        class CascadeClassifier_GPU;
    }
    namespace ocl {
        class OclCascadeClassifier;
    }
}

class CascadeClassifierDetector : public Detector
{
    Q_OBJECT
public:
    CascadeClassifierDetector(const QString &filename, QObject *parent=0);

    QString name() const;
    QList<cv::Rect> detect(const cv::Mat& image) const;

private:
    DetectorParameterList createParameters() const;
    QList<cv::Rect> filterResults(const QList<cv::Rect> &objects) const;

    QList<cv::Rect> detectMultiScale(const cv::Mat& image, cv::Size minSize, cv::Size maxSize, double scaleFactor, int minNeighbors) const;
    QList<cv::Rect> detectMultiScale_CUDA(const cv::Mat& image, cv::Size minSize, cv::Size maxSize, double scaleFactor, int minNeighbors) const;
    QList<cv::Rect> detectMultiScale_OCL(const cv::Mat& image, cv::Size minSize, cv::Size maxSize, double scaleFactor, int minNeighbors) const;

    cv::CascadeClassifier* m_classifier;
    mutable cv::gpu::CascadeClassifier_GPU* m_classifier_CUDA;
    mutable cv::ocl::OclCascadeClassifier* m_classifier_OCL;
};

#endif // CASCADECLASSIFIERDETECTOR_H

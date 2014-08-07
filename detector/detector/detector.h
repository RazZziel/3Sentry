#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>
#include <QtCore>
#include <cv.h>
#include "detectorparameter.h"

class Detector : public QObject
{
    Q_OBJECT
public:
    Detector(QObject *parent=0);

    virtual QString name() =0;
    virtual QList<cv::Rect> detect(const cv::Mat& image) const =0;

    void setEnabled(bool enabled);
    bool isEnabled();

    DetectorParameterMap getParameters() const;
    void setParameter(const QString &name, const QVariant &value);

    void loadParameterValues();

protected:
    virtual DetectorParameterList createParameters() const;

    bool m_enabled;
    DetectorParameterMap m_parameters;

private:
    void init();
    void saveParameter(const QString &name, const QVariant &value);
};

#endif // DETECTOR_H

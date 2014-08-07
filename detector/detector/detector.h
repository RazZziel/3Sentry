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
    void init();

    virtual QString name() const =0;
    virtual QList<cv::Rect> detect(const cv::Mat& image) const =0;

    void setEnabled(bool enabled);
    bool isEnabled();

    DetectorParameterMap &getParameters();
    void setParameter(const QString &name, const QVariant &value);

public slots:
    void loadParameterValues();
    void saveParameterValues();

protected:
    virtual DetectorParameterList createParameters() const;
    virtual QList<cv::Rect> filterResults(const QList<cv::Rect> &objects) const;

    bool m_enabled;
    DetectorParameterMap m_parameters;

private:
    QString settingsGroup();
    void saveParameter(const QString &name, const QVariant &value);
};

#endif // DETECTOR_H

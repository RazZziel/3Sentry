#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>
#include <QtCore>
#include <cv.h>
#include "parametermanager.h"

class Detector : public QObject, public ParameterOwner
{
    Q_OBJECT
public:
    Detector(QObject *parent=0);

    void init();
    ParameterManager *parameterManager();
    QString settingsGroup();

    virtual QString name() const =0;
    virtual QList<cv::Rect> detect(const cv::Mat& image) const =0;

    void setEnabled(bool enabled);
    bool isEnabled();

protected:
    virtual ParameterList createParameters() const;
    virtual QList<cv::Rect> filterResults(const QList<cv::Rect> &objects) const;

    ParameterManager *m_parameterManager;
    bool m_enabled;

private slots:
    void onParametersChanged();
};

#endif // DETECTOR_H

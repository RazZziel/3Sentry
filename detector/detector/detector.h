#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>
#include <QtCore>
#include <cv.h>
#include <QVariantMap>

class Detector : public QObject
{
    Q_OBJECT
public:
    Detector(QObject *parent=0);

    virtual QString name() =0;
    virtual QList<cv::Rect> detect(const cv::Mat& image) const =0;

    void setEnabled(bool enabled);
    bool isEnabled();

    QVariantMap getParameters() const;
    void setParameter(const QString &name, const QVariant &value);

    void loadParameters();

protected:
    bool m_enabled;
    QVariantMap m_parameters;

private:
    void saveParameter(const QString &name, const QVariant &value);
};

#endif // DETECTOR_H

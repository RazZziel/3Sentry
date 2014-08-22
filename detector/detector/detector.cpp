#include "detector.h"

Detector::Detector(QObject *parent) :
    QObject(parent),
    m_parameterManager(new ParameterManager(this, this)),
    m_enabled(false)
{
    connect(m_parameterManager, SIGNAL(parametersChanged()), SLOT(onParametersChanged()));
}

void Detector::setEnabled(bool enabled)
{
    m_enabled = enabled;
    m_parameterManager->setParameter("Enabled", enabled);
}

bool Detector::isEnabled()
{
    return m_enabled;
}

/// This must be called externally, because C++ sucks dicks and can't
/// implement correct inherited method overriding in the constructor
void Detector::init()
{
    m_parameterManager->init();
}

ParameterManager *Detector::parameterManager()
{
    return m_parameterManager;
}

QString Detector::settingsGroup()
{
    return QString("Detectors/%1/").arg(metaObject()->className());
}



ParameterList Detector::createParameters() const
{
    ParameterList list;
    list << Parameter("enabled", tr("Enabled"), Parameter::Boolean, false)

         << Parameter("limitWidth", tr("Limit width"), Parameter::Boolean, true)
         << Parameter("minWidth", tr("Minimum width"), Parameter::Integer, 10, 0, 1000)
         << Parameter("maxWidth", tr("Maximum width"), Parameter::Integer, 500, 0, 1000)

         << Parameter("limitHeight", tr("Limit height"), Parameter::Boolean, true)
         << Parameter("minHeight", tr("Minimum height"), Parameter::Integer, 10, 0, 1000)
         << Parameter("maxHeight", tr("Maximum height"), Parameter::Integer, 500, 0, 1000)

         << Parameter("limitArea", tr("Limit area"), Parameter::Boolean, false)
         << Parameter("minArea", tr("Minimum area"), Parameter::Integer, 10000, 0, 1000)
         << Parameter("maxArea", tr("Maximum area"), Parameter::Integer, 250000, 0, 1000);
    return list;
}

QList<cv::Rect> Detector::filterResults(const QList<cv::Rect> &objects) const
{
    ParameterMap &parameters = m_parameterManager->parameters();

    bool limitWidth = parameters.value("limitWidth").m_value.toBool();
    int minWidth = parameters.value("minWidth").m_value.toInt();
    int maxWidth = parameters.value("maxWidth").m_value.toInt();

    bool limitHeight = parameters.value("minHeight").m_value.toBool();
    int minHeight = parameters.value("minHeight").m_value.toInt();
    int maxHeight = parameters.value("maxHeight").m_value.toInt();

    bool limitArea = parameters.value("limitArea").m_value.toBool();
    int minArea = parameters.value("minArea").m_value.toInt();
    int maxArea = parameters.value("maxArea").m_value.toInt();

    QList<cv::Rect> list;

    foreach (const cv::Rect &object, objects)
    {
        if (limitWidth)
        {
            if (object.width < minWidth || object.width > maxWidth)
            {
                continue;
            }
        }

        if (limitHeight)
        {
            if (object.height < minHeight || object.height > maxHeight)
            {
                continue;
            }
        }

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

void Detector::onParametersChanged()
{
    m_enabled = m_parameterManager->parameters().value("enabled").m_value.toBool();
}

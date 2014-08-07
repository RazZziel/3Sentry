#include <QDebug>
#include <QSettings>
#include "detector.h"

Detector::Detector(QObject *parent) :
    QObject(parent),
    m_enabled(false)
{
}

void Detector::setEnabled(bool enabled)
{
    m_enabled = enabled;
    setParameter("Enabled", enabled);
}

bool Detector::isEnabled()
{
    return m_enabled;
}

DetectorParameterMap &Detector::getParameters()
{
    return m_parameters;
}

void Detector::setParameter(const QString &name, const QVariant &value)
{
    if (m_parameters.contains(name))
    {
        m_parameters[name].m_value = value;
        saveParameter(name, value);
    }
    else
    {
        qWarning() << "This detector doesn't have a parameter named" << name;
    }
}

/// This must be called externally, because C++ sucks dicks and can't
/// implement correct inherited method overriding in the constructor
void Detector::init()
{
    m_parameters.clear();

    DetectorParameterList list = createParameters();
    foreach (const DetectorParameter &parameter, list)
    {
        m_parameters.insert(parameter.m_code, parameter);
    }

    loadParameterValues();
}

void Detector::loadParameterValues()
{
    QSettings settings;
    settings.beginGroup(settingsGroup());
    {
        foreach (const QString &key, m_parameters.keys())
        {
            DetectorParameter &parameter = m_parameters[key];
            parameter.m_value = settings.value(parameter.m_code, parameter.m_defaultValue);
        }
    }
    settings.endGroup();

    m_enabled = m_parameters.value("enabled").m_value.toBool();
}

void Detector::saveParameterValues()
{
    QSettings settings;
    settings.beginGroup(settingsGroup());
    {
        foreach (const QString &key, m_parameters.keys())
        {
            DetectorParameter &parameter = m_parameters[key];
            settings.setValue(parameter.m_code, parameter.m_value);
        }
    }
    settings.endGroup();

    m_enabled = m_parameters.value("enabled").m_value.toBool();
}

QString Detector::settingsGroup()
{
    return QString("Detectors/%1/").arg(metaObject()->className());
}

void Detector::saveParameter(const QString &name, const QVariant &value)
{
    QSettings settings;
    settings.beginGroup(settingsGroup());
    settings.setValue(name, value);
    settings.endGroup();
}

DetectorParameterList Detector::createParameters() const
{
    DetectorParameterList list;
    list << DetectorParameter("enabled", tr("Enabled"), DetectorParameter::Boolean, false)

         << DetectorParameter("limitWidth", tr("Limit width"), DetectorParameter::Boolean, true)
         << DetectorParameter("minWidth", tr("Minimum width"), DetectorParameter::Integer, 100, 0, 1000)
         << DetectorParameter("maxWidth", tr("Maximum width"), DetectorParameter::Integer, 500, 0, 1000)

         << DetectorParameter("limitHeight", tr("Limit height"), DetectorParameter::Boolean, true)
         << DetectorParameter("minHeight", tr("Minimum height"), DetectorParameter::Integer, 100, 0, 1000)
         << DetectorParameter("maxHeight", tr("Maximum height"), DetectorParameter::Integer, 500, 0, 1000)

         << DetectorParameter("limitArea", tr("Limit area"), DetectorParameter::Boolean, false)
         << DetectorParameter("minArea", tr("Minimum height"), DetectorParameter::Integer, 10000, 0, 1000)
         << DetectorParameter("maxArea", tr("Maximum height"), DetectorParameter::Integer, 250000, 0, 1000);
    return list;
}

QList<cv::Rect> Detector::filterResults(const QList<cv::Rect> &objects) const
{
    bool limitWidth = m_parameters.value("limitWidth").m_value.toBool();
    int minWidth = m_parameters.value("minWidth").m_value.toInt();
    int maxWidth = m_parameters.value("maxWidth").m_value.toInt();

    bool limitHeight = m_parameters.value("minHeight").m_value.toBool();
    int minHeight = m_parameters.value("minHeight").m_value.toInt();
    int maxHeight = m_parameters.value("maxHeight").m_value.toInt();

    bool limitArea = m_parameters.value("limitArea").m_value.toBool();
    int minArea = m_parameters.value("minArea").m_value.toInt();
    int maxArea = m_parameters.value("maxArea").m_value.toInt();

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

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

DetectorParameterMap Detector::getParameters() const
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

void Detector::init()
{
    DetectorParameterList list = createParameters();
    foreach (const DetectorParameter &parameter, list)
    {
        m_parameters.insert(parameter.m_code, parameter);
    }

    loadParameterValues();
}

void Detector::loadParameterValues()
{
    m_parameters.clear();

    QSettings settings;
    settings.beginGroup(QString("Detectors/%1/").arg(staticMetaObject.className()));
    {
        foreach (const QString &key, m_parameters.keys())
        {
            DetectorParameter &parameter = m_parameters[key];
            parameter.m_value = settings.value(parameter.m_code);
        }
    }
    settings.endGroup();

    m_enabled = m_parameters.value("Enabled").m_value.toBool();
}

void Detector::saveParameter(const QString &name, const QVariant &value)
{
    QSettings settings;
    settings.beginGroup(QString("Detectors/%1/").arg(staticMetaObject.className()));
    settings.setValue(name, value);
    settings.endGroup();
}

DetectorParameterList Detector::createParameters() const
{
    DetectorParameterList list;
    list << DetectorParameter("enabled", tr("Enabled"), DetectorParameter::Boolean, false);
    return list;
}

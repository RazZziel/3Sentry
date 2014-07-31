#include <QDebug>
#include <QSettings>
#include "detector.h"

Detector::Detector(QObject *parent) :
    QObject(parent),
    m_enabled(false)
{
    loadParameters();
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

QVariantMap Detector::getParameters() const
{
    return m_parameters;
}

void Detector::setParameter(const QString &name, const QVariant &value)
{
    if (m_parameters.contains(name))
    {
        m_parameters[name] = value;
        saveParameter(name, value);
    }
    else
    {
        qWarning() << "This detector doesn't have a parameter named" << name;
    }
}

void Detector::loadParameters()
{
    m_parameters.clear();

    QSettings settings;
    settings.beginGroup(QString("Detectors/%1/").arg(staticMetaObject.className()));
    foreach (const QString &key, settings.childKeys())
    {
        m_parameters.insert(key, settings.value(key));
    }
    settings.endGroup();

    m_enabled = m_parameters.value("Enabled").toBool();
}

void Detector::saveParameter(const QString &name, const QVariant &value)
{
    QSettings settings;
    settings.beginGroup(QString("Detectors/%1/").arg(staticMetaObject.className()));
    settings.setValue(name, value);
    settings.endGroup();
}

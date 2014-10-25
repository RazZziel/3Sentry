#include "parametermanager.h"
#include <QSettings>
#include <QDebug>

ParameterManager::ParameterManager(ParameterOwner *parameterOwner, QObject *parent) :
    QObject(parent),
    m_parameterOwner(parameterOwner)
{
}

ParameterMap &ParameterManager::parameters()
{
    return m_parameters;
}

QVariant ParameterManager::value(const QString &key) const
{
    return m_parameters.value(key).m_value;
}

/// This must be called externally, because C++ sucks dicks and can't
/// implement correct inherited method overriding in the constructor
void ParameterManager::init()
{
    m_parameters.clear();

    ParameterList list = m_parameterOwner->createParameters();
    foreach (const Parameter &parameter, list)
    {
        m_parameters.insert(parameter.m_code, parameter);
    }

    loadParameterValues();
}

void ParameterManager::setParameter(const QString &name, const QVariant &value)
{
    if (m_parameters.contains(name))
    {
        qDebug() << "Setting" << name << "=" << value;
        m_parameters[name].m_value = value;
        saveParameter(name, value);
    }
    else
    {
        qWarning() << "This entity doesn't have a parameter named" << name;
    }

    emit parameterChanged(name, value);
    emit parametersChanged();
}

void ParameterManager::saveParameter(const QString &name, const QVariant &value)
{
    QSettings settings;
    settings.beginGroup(m_parameterOwner->settingsGroup());
    settings.setValue(name, value);
    settings.endGroup();
}

void ParameterManager::loadParameterValues()
{
    QSettings settings;
    settings.beginGroup(m_parameterOwner->settingsGroup());
    {
        foreach (const QString &key, m_parameters.keys())
        {
            Parameter &parameter = m_parameters[key];
            parameter.m_value = settings.value(parameter.m_code, parameter.m_defaultValue);
        }
    }
    settings.endGroup();

    emit parametersChanged();
}

void ParameterManager::saveParameterValues()
{
    QSettings settings;
    settings.beginGroup(m_parameterOwner->settingsGroup());
    {
        foreach (const QString &key, m_parameters.keys())
        {
            Parameter &parameter = m_parameters[key];
            settings.setValue(parameter.m_code, parameter.m_value);
        }
    }
    settings.endGroup();

    emit parametersChanged();
}

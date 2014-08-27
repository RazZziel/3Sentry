#include "parameter.h"

Parameter::Parameter() {}

Parameter::Parameter(const QString &code,
                     const QString &name,
                     Type type,
                     const QVariant &defaultValue,
                     const QVariant &minValue,
                     const QVariant &maxValue) :
    m_code(code),
    m_name(name),
    m_type(type),
    m_value(defaultValue),
    m_defaultValue(defaultValue),
    m_minValue(minValue),
    m_maxValue(maxValue)
{
}

Parameter Parameter::selection(const QString &code,
                               const QString &name,
                               const QVariantMap &options,
                               const QVariant &defaultValue)
{
    Parameter parameter(code, name, Selection, defaultValue);
    parameter.m_options = options;
    return parameter;
}

#include "detectorparameter.h"

DetectorParameter::DetectorParameter() {}

DetectorParameter::DetectorParameter(const QString &code,
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

#ifndef DETECTORPARAMETER_H
#define DETECTORPARAMETER_H

#include <QVariant>

class DetectorParameter
{
public:
    enum Type {
        Unknown,
        Boolean,
        Integer,
        Real,
        String,
        Color,
    };

    DetectorParameter();
    DetectorParameter(const QString &code,
                      const QString &name,
                      Type type,
                      const QVariant &defaultValue,
                      const QVariant &minValue = QVariant(),
                      const QVariant &maxValue = QVariant());

    QString m_code;
    QString m_name;
    Type m_type;

    QVariant m_value;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;
};

typedef QMap<QString, DetectorParameter> DetectorParameterMap;
typedef QList<DetectorParameter> DetectorParameterList;

#endif // DETECTORPARAMETER_H

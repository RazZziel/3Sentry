#ifndef PARAMETER_H
#define PARAMETER_H

#include <QVariant>

class Parameter
{
public:
    enum Type {
        Unknown,
        Boolean,
        Integer,
        Real,
        String,
        Color,
        Selection
    };

    Parameter();
    Parameter(const QString &code,
              const QString &name,
              Type type,
              const QVariant &defaultValue,
              const QVariant &minValue = QVariant(),
              const QVariant &maxValue = QVariant());
    static Parameter selection(const QString &code,
                               const QString &name,
                               const QVariantMap &options,
                               const QVariant &defaultValue = QVariant());

    QString m_code;
    QString m_name;
    Type m_type;

    QVariant m_value;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;

    QVariantMap m_options;
};

typedef QMap<QString, Parameter> ParameterMap;
typedef QList<Parameter> ParameterList;

#endif // PARAMETER_H

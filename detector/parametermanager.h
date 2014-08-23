#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QObject>
#include "parameter.h"

class ParameterOwner
{
public:
    virtual QString settingsGroup() =0;
    virtual ParameterList createParameters() const =0;
};

class ParameterManager : public QObject
{
    Q_OBJECT
public:
    explicit ParameterManager(ParameterOwner *parameterOwner, QObject *parent = 0);

    ParameterMap &parameters();
    QVariant value(const QString &key) const;
    void setParameter(const QString &name, const QVariant &value);
    void saveParameter(const QString &name, const QVariant &value);

public slots:
    void init();
    void loadParameterValues();
    void saveParameterValues();

protected:
    ParameterMap m_parameters;
    ParameterOwner *m_parameterOwner;

signals:
    void parametersChanged();
};

#endif // PARAMETERMANAGER_H

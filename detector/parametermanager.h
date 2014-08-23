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

public slots:
    void init();
    void loadParameterValues();
    void saveParameterValues();
    void setParameter(const QString &name, const QVariant &value);

protected:
    ParameterMap m_parameters;
    ParameterOwner *m_parameterOwner;

private:
    void saveParameter(const QString &name, const QVariant &value);

signals:
    void parametersChanged();
    void parameterChanged(const QString &name, const QVariant &value);
};

#endif // PARAMETERMANAGER_H

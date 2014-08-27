#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include <QWidget>

class Parameter;

namespace Ui {
class ParameterWidget;
}

class ParameterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterWidget(Parameter &detectorParameter, QWidget *parent = 0);
    ~ParameterWidget();

private:
    Ui::ParameterWidget *ui;
    Parameter &m_parameter;

private slots:
    void load();
    void save();

    void on_btnReset_clicked();

signals:
    void valueChanged(const QString &code, const QVariant &value);
};

#endif // PARAMETERWIDGET_H

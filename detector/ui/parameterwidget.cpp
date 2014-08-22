#include "parameterwidget.h"
#include "ui_parameterwidget.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>

#include "parameter.h"
#include "util.h"

ParameterWidget::ParameterWidget(Parameter &detectorParameter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ParameterWidget),
    m_detectorParameter(detectorParameter)
{
    ui->setupUi(this);

    load();
}

ParameterWidget::~ParameterWidget()
{
    delete ui;
}

void ParameterWidget::load()
{
    Util::clearLayout(ui->horizontalLayout, 1);

    QWidget *widget = NULL;

    switch (m_detectorParameter.m_type)
    {
    case Parameter::Unknown:
        break;

    case Parameter::Boolean:
    {
        QCheckBox *checkBox = new QCheckBox(this);
        checkBox->setChecked(m_detectorParameter.m_value.toBool());
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(save()));
        widget = checkBox;
    }
        break;

    case Parameter::Integer:
    {
        QSpinBox *spinBox = new QSpinBox(this);
        spinBox->setMinimum(m_detectorParameter.m_minValue.toInt());
        spinBox->setMaximum(m_detectorParameter.m_maxValue.toInt());
        spinBox->setValue(m_detectorParameter.m_value.toInt());
        connect(spinBox, SIGNAL(valueChanged(int)), SLOT(save()));
        widget = spinBox;
    }
        break;

    case Parameter::Real:
    {
        QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox(this);
        doubleSpinBox->setMinimum(m_detectorParameter.m_minValue.toInt());
        doubleSpinBox->setMaximum(m_detectorParameter.m_maxValue.toInt());
        doubleSpinBox->setValue(m_detectorParameter.m_value.toDouble());
        connect(doubleSpinBox, SIGNAL(valueChanged(double)), SLOT(save()));
        widget = doubleSpinBox;
    }
        break;

    case Parameter::String:
    {
        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setText(m_detectorParameter.m_value.toString());
        connect(lineEdit, SIGNAL(textChanged(QString)), SLOT(save()));
        widget = lineEdit;
    }
        break;

    case Parameter::Color:
    {
        QPushButton *pushButton = new QPushButton(tr("Select"), this);
        connect(pushButton, SIGNAL(clicked()), SLOT(save()));
        widget = pushButton;
    }
        break;
    }

    if (widget)
    {
        ui->horizontalLayout->addWidget(widget);
        adjustSize();
    }
}

void ParameterWidget::save()
{
    QWidget *widget = dynamic_cast<QWidget*>(sender());
    Q_ASSERT(widget != NULL);

    switch (m_detectorParameter.m_type)
    {
    case Parameter::Unknown:
        break;

    case Parameter::Boolean:
    {
        QCheckBox *checkBox = dynamic_cast<QCheckBox*>(widget);
        Q_ASSERT(checkBox != NULL);

        m_detectorParameter.m_value = checkBox->isChecked();
    }
        break;

    case Parameter::Integer:
    {
        QSpinBox *spinBox = dynamic_cast<QSpinBox*>(widget);
        Q_ASSERT(spinBox != NULL);

        m_detectorParameter.m_value = spinBox->value();
    }
        break;

    case Parameter::Real:
    {
        QDoubleSpinBox *doubleSpinBox = dynamic_cast<QDoubleSpinBox*>(widget);
        Q_ASSERT(doubleSpinBox != NULL);

        m_detectorParameter.m_value = doubleSpinBox->value();
    }
        break;

    case Parameter::String:
    {
        QLineEdit *lineEdit = dynamic_cast<QLineEdit*>(widget);
        Q_ASSERT(lineEdit != NULL);

        m_detectorParameter.m_value = lineEdit->text();
    }
        break;

    case Parameter::Color:
    {
        QPushButton *pushButton = dynamic_cast<QPushButton*>(widget);
        Q_ASSERT(pushButton != NULL);

        QColorDialog *d = new QColorDialog(m_detectorParameter.m_value.value<QColor>(), this);
        d->exec();
        m_detectorParameter.m_value = d->selectedColor();
    }
        break;
    }

    emit dataChanged();
}

void ParameterWidget::on_btnReset_clicked()
{
    m_detectorParameter.m_value = m_detectorParameter.m_defaultValue;
    load();
    emit dataChanged();
}

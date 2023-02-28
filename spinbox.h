#ifndef SPINBOX_H
#define SPINBOX_H

#include <QSpinBox>

class SpinBox : public QSpinBox
{
public:
    SpinBox(QWidget* parent);
protected:
    QValidator::State validate(QString& input, int& pos) const override;
    bool m_odd;
};

class OddSpinBox : public SpinBox
{
public:
    OddSpinBox(QWidget* parent);
};

class EvenSpinBox : public SpinBox
{
public:
    EvenSpinBox(QWidget* parent);
};

#endif // SPINBOX_H

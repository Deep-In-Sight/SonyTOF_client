#include "spinbox.h"

SpinBox::SpinBox(QWidget* parent) : QSpinBox(parent)
{

}

QValidator::State SpinBox::validate(QString& input, int& pos) const {
    QValidator::State state = QSpinBox::validate(input, pos);
    if (state == QValidator::Acceptable) {
        if ((input.toInt() % 2 == 1) && m_odd) {
            return QValidator::Acceptable;
        } else {
            return QValidator::Invalid;
        }
    } else {
        return state;
    }
}

OddSpinBox::OddSpinBox(QWidget* parent) : SpinBox(parent) {
    m_odd = true;
}

EvenSpinBox::EvenSpinBox(QWidget* parent) : SpinBox(parent) {
    m_odd = false;
}

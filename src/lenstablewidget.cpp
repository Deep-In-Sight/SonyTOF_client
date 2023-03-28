#include "lenstablewidget.h"
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QDebug>
#include <QHeaderView>
#include <QAbstractScrollArea>

LensTableWidget::LensTableWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setWindowTitle("Lens Parameters");
    QHBoxLayout* layout = new QHBoxLayout(this);
    tableWidget = new QTableWidget(1, 6, this);
    layout->addWidget(tableWidget);


    QStringList colName = {"fx", "fy", "pw", "py", "cx", "cy"};
    tableWidget->setHorizontalHeaderLabels(colName);

    QHeaderView* header = tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->resizeColumnsToContents();
    tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
//    this->resize(tableWidget->sizeHint());

    connect(tableWidget, &QTableWidget::itemChanged, this, &LensTableWidget::onItemChanged);
}


void LensTableWidget::setDefaultParams() {
    disconnect(tableWidget, &QTableWidget::itemChanged, this, &LensTableWidget::onItemChanged);
    float params[9] = {7.3e-3, 7.3e-3, 10e-6, 10e-6, 320.0, 240.0};
    for (int i = 0; i < tableWidget->columnCount(); i++) {
        QVariant v(params[i]);
        QTableWidgetItem* valItem = new QTableWidgetItem();
        valItem->setData(Qt::EditRole, v);
        tableWidget->setItem(0, i, valItem);
    }
    float fx_ = params[0] / params[2]; // fx/px
    float fy_ = params[1] / params[3]; // fx/px
    float cx = params[4];
    float cy = params[5];
    connect(tableWidget, &QTableWidget::itemChanged, this, &LensTableWidget::onItemChanged);
    emit paramsChanged(fx_, fy_, cx, cy);
}

void LensTableWidget::onItemChanged(QTableWidgetItem* item) {
    float params[9];
    for (int i = 0; i < tableWidget->columnCount(); i++) {
        QTableWidgetItem* item = tableWidget->item(0, i);
        float val = item->data(Qt::DisplayRole).value<float>();
        params[i] = val;
//        qDebug() << val;
    }
    float fx_ = params[0] / params[2]; // fx/px
    float fy_ = params[1] / params[3]; // fx/px
    float cx = params[4];
    float cy = params[5];
    emit paramsChanged(fx_, fy_, cx, cy);
}

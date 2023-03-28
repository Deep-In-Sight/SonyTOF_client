#ifndef LENSTABLEWIDGET_H
#define LENSTABLEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTableWidget>

class LensTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LensTableWidget(QWidget *parent = nullptr);
    void setDefaultParams();

signals:
    void paramsChanged(float fx, float fy, float cx, float cy);

private slots:
    void onItemChanged(QTableWidgetItem *item);

private:
    QTableWidget* tableWidget;

};

#endif // LENSTABLEWIDGET_H

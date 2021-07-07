#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QMutex>
#include <vector>
#include "i2c.h"
#include "freqmod.h"
#include "calculation.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void socket_Read_Data();

    void on_btn_Video_clicked();

    void on_btn_picture_clicked();

    void on_medianCheckBox_stateChanged(int arg1);

    void on_colorMapCheckBox_stateChanged(int arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_fmodBox_returnPressed();

    void on_lineEdit_offset_returnPressed();

    void on_lineEdit_intgtime_returnPressed();

private:
    void grabFrame();
    void addColorBar();
    void updateFmod();
    void writeReg(i2cReg& r);
    void setIntegrationTime(int time);

    Ui::MainWindow *ui;

    QString host;
    int port;
    QTcpSocket *socket;
    i2c* i2c_client;
    modSetting* modSet;
    QByteArray buffer;

    QImage qimg;
    QGraphicsScene* scene = new QGraphicsScene;

    int width, height;
    void imageShow(QByteArray&);
};
#endif // MAINWINDOW_H

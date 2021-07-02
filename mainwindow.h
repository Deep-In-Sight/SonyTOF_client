#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QMutex>
#include <vector>

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

private slots:
    void socket_Read_Data();

    void on_btn_Video_clicked();

    void on_btn_picture_clicked();

    void on_medianCheckBox_stateChanged(int arg1);

    void on_colorMapCheckBox_stateChanged(int arg1);

    void on_comboBox_currentIndexChanged(int index);

private:
    void grabFrame();
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QByteArray buffer;

    QImage qimg;
    QGraphicsScene* scene = new QGraphicsScene;

    int width, height;
    void imageShow(QByteArray&);
};
#endif // MAINWINDOW_H

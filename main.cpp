#include "mainwindow.h"
#include "startupdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QString host;
    int port;
    QApplication a(argc, argv);

    StartUpDialog* dialog;
    MainWindow *w;

    dialog = new StartUpDialog;
    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        dialog->getHostPort(host, port);
        w = new MainWindow(nullptr, host, port);
        w->show();
    } else {
        return -1;
    }

    return a.exec();
}

#include "mainwindow.h"
#include "startupdialog.h"

#include <QSplashScreen>
#include <QApplication>

int main(int argc, char *argv[])
{
    QString host;
    int port;
    QApplication a(argc, argv);

    QSplashScreen splash(QPixmap("logo_small.png"));
    StartUpDialog* dialog;
    MainWindow *w;

    dialog = new StartUpDialog;
    dialog->exec();

    if (dialog->result() == QDialog::Accepted) {
        dialog->getHostPort(host, port);
        splash.show();
        a.processEvents();
        w = new MainWindow(nullptr, host, port, &splash);
        w->show();
        splash.finish(w);
    } else {
        return -1;
    }

    return a.exec();
}

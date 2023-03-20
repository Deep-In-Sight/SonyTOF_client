#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>

namespace Ui {
class StartUpDialog;
}

class StartUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartUpDialog(QWidget *parent = 0);
    ~StartUpDialog();

    void getHostPort(QString &host, int &port);

private:
    Ui::StartUpDialog *ui;
};

#endif // STARTUPDIALOG_H

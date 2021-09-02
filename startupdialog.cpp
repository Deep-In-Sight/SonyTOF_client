#include "startupdialog.h"
#include "ui_startupdialog.h"

StartUpDialog::StartUpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartUpDialog)
{
    ui->setupUi(this);
}

StartUpDialog::~StartUpDialog()
{
    delete ui;
}

void StartUpDialog::getHostPort(QString &host, int &port) {
    host = ui->lineEdit_host->text();
    port = ui->lineEdit_port->text().toUInt();
}

#include "pclconfig.h"
#include "ui_pclconfig.h"
#include "pclviewer.h"

pclconfig::pclconfig(QWidget *parent, PCLViewer* viewer)
    : QWidget{parent},
      ui(new Ui::pclconfig_widget)
{
    ui->setupUi(this);
    connect(this, &pclconfig::resetView, viewer, &PCLViewer::resetView);
    connect(this, &pclconfig::downsamplerateChanged, viewer, &PCLViewer::updateDownSampling);
}

pclconfig::~pclconfig() {
    delete ui;
}



void pclconfig::on_pushButton_resetView_clicked()
{
    emit resetView();
}


void pclconfig::on_spinBox_point_downsampling_valueChanged(int arg1)
{
    emit downsamplerateChanged(arg1);
}


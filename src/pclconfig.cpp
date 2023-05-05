#include "pclconfig.h"
#include "pclviewer.h"
#include "sliderwidget.h"

#include <QDebug>

PclConfig::PclConfig(QWidget *parent, PCLViewer* viewer)
    : QWidget{parent}, m_viewer(viewer)
{
    setupUI();
    connectSignals();

    viewer->viewer->getCameraParameters(camera);
}

void PclConfig::setupUI() {
    QVBoxLayout* layout0 = new QVBoxLayout(this);
    downsample_slider = new SliderWidget(this, "Downsample Factor",0, 0, 4, 1.0);
    view_group = new QGroupBox(this);
    reset_button = new QPushButton(this);
    changeAlgo_checkbox = new QCheckBox(this);
    reset_button->setText("Reset view");
    changeAlgo_checkbox->setText("D equal Z");
    changeAlgo_checkbox->setChecked(true);
    lookAtCenter_checkbox = new QCheckBox(this);
    lookAtCenter_checkbox->setText("Look At Center");
    lookAtCenter_checkbox->setChecked(true);
    layout0->addWidget(downsample_slider);
    layout0->addWidget(changeAlgo_checkbox);
    layout0->addWidget(lookAtCenter_checkbox);
    layout0->addWidget(view_group);
    layout0->addWidget(reset_button);


    QVBoxLayout* layout1 = new QVBoxLayout(view_group);
    QStringList labels({"pos_x", "pos_y", "pos_z",
                       "view_x", "view_y", "view_z",
                       "up_x", "up_y", "up_z",
                       "fovy"});
    for (int i = 0; i < labels.count(); i++) {
        auto label = labels[i];
        auto slider = new SliderWidget(view_group, label, i);
        layout1->addWidget(slider);
        sliders.append(slider);
    }
}

void PclConfig::connectSignals() {
    //receive from ui
    connect(downsample_slider, &SliderWidget::newValue, this, &PclConfig::onDownSampleRateChanged);
    for (int i = 0; i < sliders.count(); i++) {
        connect(sliders[i], &SliderWidget::newValue, this, &PclConfig::onViewGroupChanged);
    }
    connect(reset_button, &QPushButton::clicked, this, &PclConfig::onResetButtonClicked);
    connect(changeAlgo_checkbox, &QCheckBox::stateChanged, this, &PclConfig::onAlgoCheckboxStateChanged);
    connect(lookAtCenter_checkbox, &QCheckBox::stateChanged, this, &PclConfig::onLookAtCenterCheckboxStateChanged);

    // receive from viewer
    connect(m_viewer, &PCLViewer::viewerDragged, this, &PclConfig::onViewerDragged);

    //transmit
    connect(this, &PclConfig::downSampleRateChanged, m_viewer, &PCLViewer::onDownSampleRateChanged);
    connect(this, &PclConfig::viewChanged, m_viewer, &PCLViewer::onViewChanged);
    connect(this, &PclConfig::viewReset, m_viewer, &PCLViewer::onViewReset);
    connect(this, &PclConfig::algoChanged, m_viewer, &PCLViewer::onAlgoChanged);
    connect(this, &PclConfig::lookAtChanged, m_viewer, &PCLViewer::onLookAtChanged);

}

void PclConfig::onViewGroupChanged(float val, int slider_id) {
    qDebug() << "slided";
    switch(slider_id) {
        //camera position
        case 0: camera.pos[0] = val; break;
        case 1: camera.pos[1] = val; break;
        case 2: camera.pos[2] = val; break;
        //camera look at
        case 3: camera.focal[0] = val; break;
        case 4: camera.focal[1] = val; break;
        case 5: camera.focal[2] = val; break;
        //camera up vector
        case 6: camera.view[0] = val; break;
        case 7: camera.view[1] = val; break;
        case 8: camera.view[2] = val; break;
        //camera field of view
        case 9: camera.fovy = val; break;
        default: break;
    }

    emit viewChanged(camera);

}

void PclConfig::onViewerDragged(pcl::visualization::Camera& camera) {
    qDebug() << "dragged";
    sliders[0]->setFloatValue(camera.pos[0]);
    sliders[1]->setFloatValue(camera.pos[1]);
    sliders[2]->setFloatValue(camera.pos[2]);
    sliders[3]->setFloatValue(camera.focal[0]);
    sliders[4]->setFloatValue(camera.focal[1]);
    sliders[5]->setFloatValue(camera.focal[2]);
    sliders[6]->setFloatValue(camera.view[0]);
    sliders[7]->setFloatValue(camera.view[1]);
    sliders[8]->setFloatValue(camera.view[2]);
    sliders[9]->setFloatValue(camera.fovy);
}

void PclConfig::onResetButtonClicked()
{
    emit viewReset();
}

void PclConfig::onDownSampleRateChanged(float val, int ignored)
{
    int factor = (int)val;
    emit downSampleRateChanged(factor);
}

void PclConfig::onAlgoCheckboxStateChanged(int state) {
    bool isDEqualZ;
    if (state == Qt::Checked) {
        isDEqualZ = true;
    } else {
	isDEqualZ = false;
    }
    emit algoChanged(isDEqualZ);
}

void PclConfig::onLookAtCenterCheckboxStateChanged(int state) {
    bool lookAtCenter;
    if (state == Qt::Checked) {
	  lookAtCenter = true;
    } else {
	  lookAtCenter = false;
    }
    emit lookAtChanged(lookAtCenter);
}


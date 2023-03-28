#ifndef PCLCONFIG_H
#define PCLCONFIG_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>

#include "sliderwidget.h"
#include "pcl/visualization/common/common.h"

class PCLViewer;

class PclConfig : public QWidget
{
    Q_OBJECT
public:
    explicit PclConfig(QWidget *parent = nullptr, PCLViewer* viewer = nullptr);

signals:
    void downSampleRateChanged(int& factor);
    void algoChanged(int algo);
    void viewChanged(pcl::visualization::Camera& camera);
    void viewReset();

private slots:
    void onResetButtonClicked();
    void onAlgoCheckboxStateChanged(int state);
    void onDownSampleRateChanged(float val, int ignored);
    void onViewGroupChanged(float val, int slider_id);
    void onViewerDragged(pcl::visualization::Camera& camera);

private:
    void setupUI();
    void connectSignals();


private:
    PCLViewer* m_viewer;

    SliderWidget* downsample_slider;
    QGroupBox* view_group;
    QVector<SliderWidget*> sliders;
    QPushButton* reset_button;
    QCheckBox* changeAlgo_checkbox;

    pcl::visualization::Camera camera;
};

#endif // PCLCONFIG_H

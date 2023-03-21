#include "pclviewer.h"
#include "imagerthread.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QTimer>
#include <QWidget>
#include <QHBoxLayout>

#if VTK_MAJOR_VERSION > 8
#include <vtkGenericOpenGLRenderWindow.h>
#endif

#define WIDTH 640
#define HEIGHT 480

PCLViewer::PCLViewer (QWidget *parent) :
  QWidget (parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    qvtkWidget = new PCLQVTKWidget(this);
    layout->addWidget(qvtkWidget);


  // Setup the cloud pointer
  cloud.reset (new PointCloudT(WIDTH, HEIGHT, PointT(0.0,0.0,0.0,0,0,0,0)));
  // The number of points in the cloud
//  cloud->resize (WIDTH*HEIGHT);

  m_scale_x = new float[WIDTH];
  m_scale_y = new float[HEIGHT];

  // Set up the QVTK window  
#if VTK_MAJOR_VERSION > 8
  auto renderer = vtkSmartPointer<vtkRenderer>::New();
  auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  viewer.reset(new pcl::visualization::PCLVisualizer(renderer, renderWindow, "viewer", false));
  qvtkWidget->setRenderWindow(viewer->getRenderWindow());
  viewer->setupInteractor(qvtkWidget->interactor(), qvtkWidget->renderWindow());
#else
  viewer.reset(new pcl::visualization::PCLVisualizer("viewer", false));
  ui->qvtkWidget->SetRenderWindow(viewer->getRenderWindow());
  viewer->setupInteractor(ui->qvtkWidget->GetInteractor(), ui->qvtkWidget->GetRenderWindow());
#endif

  viewer->addPointCloud (cloud, "cloud");
  viewer->resetCamera ();
  
  refreshView();

//  timer = new QTimer(this);
//  timer->setInterval(30);
//  timer->callOnTimeout([=]() {
//    this->updateCloud();
//    this->refreshView();
//  });
//  timer->start();

}

void PCLViewer::setZscale(int fmod_mhz) {
    float range = 300.0/2/fmod_mhz;
    m_scale_z = range / 32767;
}

void PCLViewer::setLensIntrinsic(float cx, float cy, float fx, float fy) {
    m_cx = cx;
    m_cy = cy;
    m_fx = fx;
    m_fy = fy;

    for (int u = 0; u < WIDTH; u++) {
        m_scale_x[u] = (u - cx)/ fx;
    }

    for (int v = 0; v < HEIGHT; v++) {
        m_scale_y[v] = (v - cy)/fy;
    }
}

void PCLViewer::setColorStyle(int colorStyleId) {
    m_colorStyle = colorStyleId;
}

void
PCLViewer::updateCloud(QByteArray newDepthMap, int mode)
{
//    if (mode != POINTCLOUD_MODE)
//        return;
    static bool first_update = true;

    qDebug() << "up";
    cv::Mat phaseMap(HEIGHT, WIDTH, CV_16SC1, newDepthMap.data());
    cv::Mat phaseMap8b;
    phaseMap.convertTo(phaseMap8b, CV_8UC1, 1/128.0);
    cv::Mat colorMap;
    cv::applyColorMap(phaseMap8b, colorMap, m_colorStyle);

    for (int v = 0; v < HEIGHT; v++) {
        for (int u = 0; u < WIDTH; u++) {
            int phase = phaseMap.at<cv::int16_t>(v, u); //row, col
            float z = m_scale_z * phase;
            cv::Vec3b& color = colorMap.at<cv::Vec3b>(v, u);
            PointT& p = (*cloud).at(u, v); //col, row
            p.x = m_scale_x[u] * z;
            p.y = m_scale_y[v] * z;
            p.z = z;
            if (z >= 0) {
                p.b = color[0];
                p.g = color[1];
                p.r = color[2];
                p.a = 255;
            } else {
                p.a = 0; //make invalidated point invisible
            }
        }
    }
    viewer->updatePointCloud(cloud, "cloud");
    refreshView();
    if (first_update) {
        first_update = false;
            viewer->resetCamera();
    }

}

void
PCLViewer::refreshView()
{
#if VTK_MAJOR_VERSION > 8
  qvtkWidget->renderWindow()->Render();
#else
  ui->qvtkWidget->update();
#endif
}

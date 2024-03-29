#include "pclviewer.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QTimer>
#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>

#if VTK_MAJOR_VERSION > 8
#include <vtkGenericOpenGLRenderWindow.h>
// #include <vtkSMPTools.h>
#endif

#include "pclconfig.h"

#define WIDTH 640
#define HEIGHT 480

PCLViewer::PCLViewer (QWidget *parent) :
    QWidget (parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    qvtkWidget = new PCLQVTKWidget(this);
    layout->addWidget(qvtkWidget);

    //    vtkSMPTools::Initialize(4);
    //    vtkSMPTools::SetBackend("Sequential");

    // Setup the cloud pointer
    m_downSample = 1;
    m_d_equal_z = true;
    m_lookAtCenter = true;
    cloud.reset (new PointCloudT(WIDTH/m_downSample, HEIGHT/m_downSample));
    min_x = min_y = min_z = 1000;
    max_x = max_y = max_z = -1000;

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
    viewer->addCoordinateSystem(0.5);
    // Assuming physical camera's origin is also world's origin
    // The following align the virtual camera with physical camera,
    // and put it 1meter behind on the optical center axis
    // like this:
    //
    //     y| /z
    //      |/__x   (physical)
    //
    //  y| /z
    //   |/__x  (virtual)
    viewer->setCameraPosition(0.0, 0.0, -1.0,
                              0.0, 0.0, 0.0,
                              0.0, 1.0, 0.0); //rotate camera straight
    viewer->setCameraFieldOfView(3.14/2); //90degree
    refreshView();

    config_widget = new PclConfig(nullptr, this);
    config_widget->setWindowTitle("Pointcloud config");
}

void PCLViewer::showEvent(QShowEvent* event) {
    config_widget->show();
}

void PCLViewer::hideEvent(QHideEvent* event) {
    config_widget->hide();
}

void PCLViewer::mouseReleaseEvent(QMouseEvent* e) {
    pcl::visualization::Camera cam;
    viewer->getCameraParameters(cam);
    emit viewerDragged(cam);
}

void PCLViewer::setZscale(int fmod_mhz) {
    float range = 300.0/2/fmod_mhz;
    m_scale_z = range / 32767;
}

void PCLViewer::setLensIntrinsic(float fx, float fy, float cx, float cy) {
    m_cx = cx;
    m_cy = cy;
    m_fx = fx;
    m_fy = fy;
}

void PCLViewer::setColorStyle(int colorStyleId) {
    m_colorStyle = colorStyleId;
}

void PCLViewer::onAlgoChanged(bool d_equal_z) {
    m_d_equal_z = d_equal_z;
}

void PCLViewer::onLookAtChanged(bool lookAtCenter) {
    m_lookAtCenter = lookAtCenter;
    qDebug() << "look at center:" << lookAtCenter;
}

void PCLViewer::updateMinMax(float x, float y, float z) {
  if (x < min_x) {
    min_x = x;
  }
  if (y < min_y) {
    min_y = y;
  }
  if (z < min_z) {
    min_z = z;
  }
  if (x > max_x) {
    max_x = x;
  }
  if (y > max_y) {
    max_y = y;
  }
  if (z > max_z) {
    max_z = z;
  }
}

void
PCLViewer::onFilterDone(QByteArray newDepthMap, int mode)
{
    static bool first_update = true;

    //    qDebug() << "up";
    cv::Mat phaseMap(HEIGHT, WIDTH, CV_16SC1, newDepthMap.data());
    cv::Mat phaseMap8b;
    phaseMap.convertTo(phaseMap8b, CV_8UC1, 1/128.0);
    cv::Mat colorMap;
    cv::applyColorMap(phaseMap8b, colorMap, m_colorStyle);

    if (m_d_equal_z) {
        for (int v = 0; v < HEIGHT; v+=m_downSample) {
            for (int u = 0; u < WIDTH; u+=m_downSample) {
                int16_t phase = phaseMap.at<int16_t>(v, u); //row, col
                float z = m_scale_z * phase;
                cv::Vec3b& color = colorMap.at<cv::Vec3b>(v, u);
                PointT& p = (*cloud).at(u/m_downSample, v/m_downSample); //col, row
                float kx = (u - m_cx) / m_fx;
                float ky = (v - m_cy) / m_fy;
                p.x = -1 * kx * z;
                p.y = -1 * ky * z;
                p.z = z;
                if (z >= 0) {
                    p.b = color[0];
                    p.g = color[1];
                    p.r = color[2];
                    p.a = 255;
                } else {
                    p.a = 0; //make invalidated point invisible
                }
		updateMinMax(p.x, p.y, p.z);
            }
        }
    } else {
        for (int v = 0; v < HEIGHT; v+=m_downSample) {
            for (int u = 0; u < WIDTH; u+=m_downSample) {
                int16_t phase = phaseMap.at<int16_t>(v, u); //row, col
                float d = m_scale_z * phase;
                cv::Vec3b& color = colorMap.at<cv::Vec3b>(v, u);
                PointT& p = (*cloud).at(u/m_downSample, v/m_downSample); //col, row
                float kx = (u - m_cx) / m_fx;
                float ky = (v - m_cy) / m_fy;
                float z = d / sqrt(1 + kx*kx + ky*ky);

                p.x = -1 * kx * z;
                p.y = -1 * ky * z;
                p.z = z;
                if (z >= 0) {
                    p.b = color[0];
                    p.g = color[1];
                    p.r = color[2];
                    p.a = 255;
                } else {
                    p.a = 0; //make invalidated point invisible
                }
		updateMinMax(p.x, p.y, p.z);
            }
        }
    }
    viewer->updatePointCloud(cloud, "cloud");
    if (m_lookAtCenter) {
        updateLookAtPoint();
    }
    refreshView();
    //    if (first_update) {
    //        first_update = false;
    //            viewer->resetCamera();
    //    }

}

void
PCLViewer::onViewChanged(pcl::visualization::Camera& c) {

    viewer->setCameraPosition(c.pos[0], c.pos[1], c.pos[2],
            c.focal[0], c.focal[1], c.focal[2],
            c.view[0], c.view[1], c.view[2]);
    refreshView();
}

void 
PCLViewer::updateLookAtPoint() {
   pcl::visualization::Camera c;
   viewer->getCameraParameters(c);
   float focal_x = (min_x + max_x)/2;
   float focal_y = (min_y + max_y)/2;
   float focal_z = (min_z + max_z)/2;
   viewer->setCameraPosition(c.pos[0], c.pos[1], c.pos[2],
		             focal_x, focal_y, focal_z,
			     c.view[0], c.view[1], c.view[2]);

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

void
PCLViewer::onViewReset() {
    viewer->setCameraPosition(0.0, 0.0, -1.0, //put the virtual camera 1 meter behind the physical camera in z direction
                              0.0, 0.0, 0.0, //point the virtual camera toward world origin
                              0.0, 1.0, 0.0); //rotate camera straight
    viewer->setCameraFieldOfView(3.14/2); //90degree
    pcl::visualization::Camera cam;
    viewer->getCameraParameters(cam);
    emit viewerDragged(cam);
    refreshView();
}

void
PCLViewer::onDownSampleRateChanged(int factor) {
    m_downSample = pow(2, factor);
    viewer->removePointCloud();
    cloud.reset (new PointCloudT(WIDTH/m_downSample, HEIGHT/m_downSample));
    viewer->addPointCloud(cloud);
}

#pragma once

#include <iostream>

// Qt
#include <QWidget>

// Point Cloud Library
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/qvtk_compatibility.h>

typedef pcl::PointXYZRGBA PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

class PCLViewer : public QWidget
{
  Q_OBJECT

public:
  explicit PCLViewer (QWidget *parent = 0);
  void setZscale(int fmod_mhz);
  void setLensIntrinsic(float cx, float cy, float fx, float fy);
  void setColorStyle(int colorStyleId);

public:
  void
  refreshView();

public slots:
  void
  updateCloud(QByteArray newDepthMap, int mode);

public:
  pcl::visualization::PCLVisualizer::Ptr viewer;
  PointCloudT::Ptr cloud;

  unsigned int red;
  unsigned int green;
  unsigned int blue;

private:
  PCLQVTKWidget* qvtkWidget;
  QTimer* timer;
  float m_scale_z;
  float* m_scale_x;
  float* m_scale_y;
  float m_cx;
  float m_cy;
  float m_fx;
  float m_fy;
  int m_colorStyle;
};

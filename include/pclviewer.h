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
//typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

class PclConfig;

class PCLViewer : public QWidget
{
  Q_OBJECT

public:
  explicit PCLViewer (QWidget *parent = 0);
  void setZscale(int fmod_mhz);

  void setColorStyle(int colorStyleId);

public:
  void
  refreshView();

public slots:

  void
  setLensIntrinsic(float fx, float fy, float cx, float cy);

  void
  onFilterDone(QByteArray newDepthMap, int mode);

  void
  onDownSampleRateChanged(int factor);

  void
  onAlgoChanged(int algo);

  void
  onViewChanged(pcl::visualization::Camera& camera);

  void
  onViewReset();

signals:
  void viewerDragged(pcl::visualization::Camera& camera);

public:
  pcl::visualization::PCLVisualizer::Ptr viewer;
  PointCloudT::Ptr cloud;

  unsigned int red;
  unsigned int green;
  unsigned int blue;

protected:
  virtual void showEvent(QShowEvent* e);
  virtual void hideEvent(QHideEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);

private:
  PCLQVTKWidget* qvtkWidget;
  PclConfig* config_widget;
  QTimer* timer;
  float m_scale_z;
  float* m_scale_x;
  float* m_scale_y;
  float m_cx;
  float m_cy;
  float m_fx;
  float m_fy;
  int m_colorStyle;
  int m_downSample;
  int m_algo;
};

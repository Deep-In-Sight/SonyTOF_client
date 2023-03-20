#ifndef FILTERTHREAD_H
#define FILTERTHREAD_H


#include <QThread>
#include <QImage>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <opencv2/core.hpp>

enum FILTER_TYPE {
    FILTER_NONE = 0,
    FILTER_MEDIAN,
    FILTER_GAUSSIAN,
    FILTER_GUIDED,
    FILTER_HYBRIDMEDIAN,
};


class FilterThread : public QThread
{
    Q_OBJECT

public:
    FilterThread(QObject *parent = 0);
    ~FilterThread();

    void run();

    void updateMedian(int& ksize);
    void updateGaussian(int& ksize, double& sigma);
    void updateGuided(int& radius, double& epsilon);

    void toggleFilter(FILTER_TYPE type, bool enable);

public slots:
    void slotFilter(QByteArray rawImage, int mode);

signals:
    void signalFilterDone(QByteArray filteredRawImage, int mode);

private:
    bool quit;
    QByteArray pRawImage;
    int pDisplayMode;

    std::list<FILTER_TYPE> m_filters;

    //median filter configs
    int m_medianSize;
    //gaussian filter configs
    int m_gaussianSize;
    double m_gaussianSigma;
    //guided filter configs
    int m_guidedRadius;
    double m_guidedEpsilon;

    QMutex mutex;
    QWaitCondition cond;
    bool cond_notified;

    void doFilter(cv::Mat& src, cv::Mat& dst, FILTER_TYPE type);
};

void hybridMedianFilter(cv::Mat& src, cv::Mat& dst, int& ksize);
#endif // FILTERTHREAD_H

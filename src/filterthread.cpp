#include "filterthread.h"
#include "imagerthread.h"
#include "profile.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

FilterThread::FilterThread(QObject* parent)
    :QThread(parent), quit(false), cond_notified(false)
{
}

FilterThread::~FilterThread() {
    quit = true;
    mutex.lock();
    cond_notified = true;
    cond.wakeOne();
    mutex.unlock();
    wait();
}

void FilterThread::slotFilter(const QByteArray rawImage, int mode) {
    QMutexLocker locker(&mutex);
    pRawImage = rawImage;
    pDisplayMode = mode;

    if (!isRunning()) {
        start();
    } else {
        cond_notified = true;
        cond.wakeOne();
    }
}

void FilterThread::run() {
    QByteArray filteredRaw;
    QSize imgSize;
    int dataType;
    cv::Mat src, dst;
    std::list< FILTER_TYPE> filters;

    QByteArray rawImage;
    DISPLAY_MODE displayMode;

    while (!quit) {
        mutex.lock();
        filters = m_filters;
        rawImage = pRawImage;
        displayMode = (DISPLAY_MODE) pDisplayMode;
        mutex.unlock();

        if (displayMode == DCS_MODE) {
            imgSize = QSize(640, 480*4);
        } else {
            imgSize = QSize(640, 480);
        }

        if (displayMode == AMPLITUDE_MODE_CLAMP ||
                displayMode == AMPLITUDE_MODE_SCALE) {
            dataType = CV_8UC1;
        } else {
            dataType = CV_16SC1;
        }

        __TIC__(ALL_FILTERS);
        src = cv::Mat(imgSize.height(), imgSize.width(), dataType, rawImage.data());
        if (displayMode == DISTANCE_MODE) {
            for (auto type: filters) {
                doFilter(src, dst, type);
                src = dst;
            }
        }
        __TOC__(ALL_FILTERS);

        filteredRaw = QByteArray::fromRawData((char*)src.data, src.rows*src.step);

        qDebug() << "filter done " << displayMode;
        emit signalFilterDone(filteredRaw, displayMode);

        mutex.lock();
        while (!cond_notified) {
            cond.wait(&mutex);
        }
        cond_notified = false;
        mutex.unlock();
    }
}

void FilterThread::toggleFilter(FILTER_TYPE type, bool enable) {
    mutex.lock();
    if (enable) {
        m_filters.push_back(type);
    } else {
        for (auto it = m_filters.begin(); it != m_filters.end(); it++) {
            if (*it == type) {
                it = m_filters.erase(it);
            }
        }
    }

    auto dbg = qDebug();
    for (auto it = m_filters.begin(); it != m_filters.end(); it++) {
        dbg << *it << " ";
    }
    dbg << Qt::endl;
    mutex.unlock();

    return;
}

void FilterThread::doFilter(cv::Mat& src, cv::Mat& dst, FILTER_TYPE type) {
    cv::Mat mask;
    switch (type) {
    case FILTER_NONE:
        dst = src;
        break;
    case FILTER_MEDIAN:
        cv::medianBlur(src, dst, m_medianSize);
        break;
    case FILTER_GAUSSIAN:
        cv::GaussianBlur(src, dst, cv::Size(m_gaussianSize, m_gaussianSize),
                         m_gaussianSigma);
        break;
    case FILTER_GUIDED:
        src = cv::Mat(src.rows, src.cols, CV_16SC1, src.data);
        src.convertTo(src, CV_32FC1, 1.0/32767);
        mask = (src < 0);
        mask.convertTo(mask, CV_32FC1, -1.0/255);
        src = src + mask;
        cv::ximgproc::guidedFilter(src, src, dst, m_guidedRadius, m_guidedEpsilon);
//        dst = dst - mask;

        dst.convertTo(dst, CV_16SC1, 32767);
        qDebug() << src.type() << " " <<  dst.type();
        break;
    case FILTER_HYBRIDMEDIAN:
//        dst = src;
        hybridMedianFilter(src, dst, m_medianSize);
        break;
    default:
        dst = src;
        qDebug() << "Filter not found";
        break;
    }
}

void FilterThread::updateMedian(int& ksize) {
    m_medianSize = ksize;

    qDebug() << "median config: ksize=" << m_medianSize;
}

void FilterThread::updateGaussian(int& ksize, double& sigma) {
    m_gaussianSize = ksize;
    m_gaussianSigma = sigma;

    qDebug() << "gaussian config: ksize=" << m_gaussianSize << " sigma=" << sigma;
}

void FilterThread::updateGuided(int& radius, double& epsilon) {
    m_guidedRadius = radius;
    m_guidedEpsilon = epsilon;

    qDebug() << "guided filter config: radius=" << m_guidedRadius << " epsilon=" << epsilon;
}

short median(std::vector<short>& v) {
    int n = v.size()/2;
    std::nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}

void hybridMedianFilter(cv::Mat& src, cv::Mat& dst, int& ksize) {
    Q_ASSERT(ksize%2 == 1);
    int pad = ksize/2;
    dst = cv::Mat(src.rows, src.cols, src.type());
    cv::copyMakeBorder(src, src, pad, pad, pad, pad, cv::BORDER_REFLECT101);
    std::vector<short> cross1(ksize*2-1);
    std::vector<short> cross2(ksize*2-1);
    std::vector<short> med(3);
    for (int y = 0; y < dst.rows; y++) {
        for (int x = 0; x < dst.cols; x++) {
            for (int k = 0; k < ksize; k++) {
                cross1[k] = src.at<short>(y+k, x+k);
                cross2[k] = src.at<short>(y+k, x+ksize/2);
            }
            for (int k = 0; k < ksize; k++) {
                if (k == ksize/2)
                    continue;
                cross1[ksize+k] = src.at<short>(y+k, x+ksize-1 - k);
                cross2[ksize+k] = src.at<short>(y+ksize/2, x+k);
            }
            med[0] = median(cross1);
            med[1] = median(cross2);
            med[2] = src.at<short>(y+ksize/2, x+ksize/2);
            dst.at<short>(y, x) = median(med);
        }
    }
}

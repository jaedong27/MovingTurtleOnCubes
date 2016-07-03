#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QThread>
#include <QMutex>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

class Camera : public QThread
{
public:
    Camera();

    cv::Mat _image;
    bool _stop;
    QMutex _mutex;
    int _camera_index;
    int _width, _height;
    cv::Mat _intrinsic_param;
    cv::Mat _distort_param;
    cv::Mat _intrinsic_inv_param;
    cv::Mat _distort_remap1;
    cv::Mat _distort_remap2;

    float _focal;
    float _focal_x,_focal_y;
    cv::Point2f _pp;

    bool open(int index);
    bool isOpen();
    bool getImage();
    void run();

private:
    cv::VideoCapture _capture;
};

#endif // CAMERA_H

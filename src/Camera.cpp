#include "Camera.h"
#include "QDebug"

Camera::Camera()
{
    _camera_index = -1;
}

bool Camera::open(int index)
{
    _image = cv::Mat(10,10,CV_8UC3);
    _capture = cv::VideoCapture(index);
    _capture.set(CV_CAP_PROP_FOURCC,CV_FOURCC('H','2','6','4'));
    _capture.set(CV_CAP_PROP_FRAME_WIDTH,1920);//2304);//1829//1200//800
    _capture.set(CV_CAP_PROP_FRAME_HEIGHT,1080);//1536); //1080//800//600
    _capture.set(CV_CAP_PROP_FPS, 1000);

    _width = 1920;
    _height = 1080;
//    qDebug() << "CV_CAP_PROP_EXPOSURE" << ":" << _capture.set(CV_CAP_PROP_EXPOSURE,-2.0f);
//    qDebug() << "CV_CAP_PROP_EXPOSURE" << ":" << _capture.get(CV_CAP_PROP_EXPOSURE);
    qDebug() << "CV_CAP_PROP_FOCUS" << ":" << _capture.set(CV_CAP_PROP_FOCUS, 0);
//    qDebug() << "CV_CAP_PROP_FOCUS" << " : " << _capture.set(CV_CAP_PROP_FOCUS, 0);
    _capture.set(CV_CAP_PROP_FOCUS, 0);

    _intrinsic_param = cv::Mat(3,3,CV_64F);
    _distort_param = cv::Mat(1,4,CV_64F);

    _focal = 1382.0f;
    _focal_x = 1383.433;
    _focal_y = 1384.312;
    _pp.x = 959.241044f;
    _pp.y = 522.888753f;

    _intrinsic_param.at<double>(0,0) = _focal; _intrinsic_param.at<double>(0,1) = 0    ; _intrinsic_param.at<double>(0,2) = _pp.x;
    _intrinsic_param.at<double>(1,0) = 0    ; _intrinsic_param.at<double>(1,1) = _focal; _intrinsic_param.at<double>(1,2) = _pp.y;
    _intrinsic_param.at<double>(2,0) = 0    ; _intrinsic_param.at<double>(2,1) = 0    ; _intrinsic_param.at<double>(2,2) = 1;

    _intrinsic_inv_param = _intrinsic_param.inv();

    _distort_param.at<double>(0,0) =  0.089712;
    _distort_param.at<double>(0,1) = -0.157344;
    _distort_param.at<double>(0,2) = -0.000022;
    _distort_param.at<double>(0,3) = -0.000450;
//    Mat view, rview, map1, map2;
//      initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
//          getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
//          imageSize, CV_16SC2, map1, map2);

    cv::initUndistortRectifyMap(_intrinsic_param,_distort_param,cv::Mat(),
                                 cv::getOptimalNewCameraMatrix(_intrinsic_param, _distort_param, cv::Size(1920,1080), 1, cv::Size(1920,1080), 0),
                                 cv::Size(_width,_height), CV_16SC2, _distort_remap1, _distort_remap2);

    return isOpen();
}

bool Camera::isOpen()
{
    return _capture.isOpened();
}

bool Camera::getImage()
{
    bool frame_valid = true;
    _mutex.lock();
    try {
        _capture >> _image; // get a new frame from webcam
        //resize(_image, _image, cv::Size(), 2, 2);
        cv::remap(_image, _image, _distort_remap1, _distort_remap2, cv::INTER_LINEAR);
        //cv::cvtColor(_image,_image,CV_BGR2RGB);
    } catch(cv::Exception& e) {
        frame_valid = false;
    }
    _mutex.unlock();
    return frame_valid;
}

void Camera::run()
{
    _stop = false;
    while(!_stop)
    {
        if(getImage())
        {
//            cv::imshow("test",_image);
//            cv::waitKey(1);
        }
        else
        {

        }
        QThread::msleep(1);
    }
}

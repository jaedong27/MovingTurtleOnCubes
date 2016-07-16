#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QSettings"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _click_index = 0;
    _point[0] = cv::Point2i(10 ,10 );
    _point[1] = cv::Point2i(200,10 );
    _point[2] = cv::Point2i(10 ,200);
    _point[3] = cv::Point2i(200,200);
    _point[4] = cv::Point2i(100,100);

    QSettings qsettings("GSCT", "Turtle");
    ui->numCameraIndex->setValue(qsettings.value("CameraIndex",0).toInt());
    ui->numWidth->setValue(qsettings.value("BaseWidth",0).toFloat());
    ui->numHeight->setValue(qsettings.value("BaseHeight",0).toFloat());
    ui->numCenterWidth->setValue(qsettings.value("CenterWidth",0).toFloat());
    ui->numCenterHeight->setValue(qsettings.value("CenterHeight",0).toFloat());
    ui->numCubeSize->setValue(qsettings.value("CubeSize",0).toFloat());

    _turtle_image = cv::imread("./turtle.png");
    if(_turtle_image.empty())
    {
        qDebug() << "Turtle Image Empty";
        exit(1);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onMouse(int evt, int x, int y, int flags, void *param)
{
    MainWindow *p = (MainWindow *)param;
    //p->HandleEvent(evt, x, y, flags);
    //qDebug() << x << y;
    if( evt == CV_EVENT_LBUTTONDOWN )
    {
        qDebug() << "Calib Down " << p->_click_index << x << y;
        p->_point[p->_click_index] = cv::Point2i(x,y);
    }
    return;
}

void MainWindow::on_butCameraOpen_clicked()
{
    int camera_index = ui->numCameraIndex->value();
    _camera.open(camera_index);
    qDebug() << _camera.isOpen();
    if(_camera.isOpen())
    {
        _camera._stop = true;
        _camera.wait();
        _camera.start();

        QSettings qsettings("GSCT", "Turtle");
        qsettings.setValue("CameraIndex",camera_index);
    }
}

void MainWindow::on_butCalibration_clicked()
{
    cv::namedWindow("Calibration");
    cv::setMouseCallback( "Calibration", onMouse, this );
    while(1)
    {
        bool end_flag = false;
        //Calibration Update;
        _camera._mutex.lock();
        cv::Mat image = _camera._image.clone();
        _camera._mutex.unlock();
        cv::line(image,_point[0],_point[1],cv::Scalar(0,0,255),1);
        cv::line(image,_point[1],_point[3],cv::Scalar(255,255,0),1);
        cv::line(image,_point[3],_point[2],cv::Scalar(255,255,0),1);
        cv::line(image,_point[2],_point[0],cv::Scalar(255,255,0),1);
        cv::circle(image,_point[0],5,cv::Scalar(0,0,255),2);
        cv::imshow("Calibration",image);
        int ch = cv::waitKey(1);
        switch(ch)
        {
            case '1':
                _click_index = 0;
            break;
            case '2':
                _click_index = 1;
            break;
            case '3':
                _click_index = 2;
            break;
            case '4':
                _click_index = 3;
            break;
            //case '5':
            //    _click_index = 4;
            //break;
            case 'e':
                end_flag = true;
            break;
        }

        if(end_flag) break;
    }

    // matching pairs
    std::vector<cv::Point3f> objectPoints;	// 3d world coordinates
    std::vector<cv::Point2f> imagePoints;	// 2d image coordinates

    float object_width  = ui->numWidth->value();
    float object_height = ui->numHeight->value();

    objectPoints.clear();
    objectPoints.push_back(cv::Point3f(0,0,0));
    objectPoints.push_back(cv::Point3f(0,object_width,0));
    objectPoints.push_back(cv::Point3f(object_height,0,0));
    objectPoints.push_back(cv::Point3f(object_height,object_width,0));

    imagePoints.clear();
    imagePoints.push_back(_point[0]);
    imagePoints.push_back(_point[1]);
    imagePoints.push_back(_point[2]);
    imagePoints.push_back(_point[3]);

    // camera parameters
    double m[] = {_camera._focal, 0, _camera._pp.x, 0, _camera._focal, _camera._pp.y, 0, 0, 1};	// intrinsic parameters
    cv::Mat A(3, 3, CV_64FC1, m);	// camera matrix

    //double d[] = {k1, k2, p1, p2};	// k1,k2: radial distortion, p1,p2: tangential distortion
    cv::Mat distCoeffs = _camera._distort_param;//(4, 1, CV_64FC1, d);

    qDebug() << "Calibration";

    // estimate camera pose
    cv::Mat rvec, tvec;	// rotation & translation vectors
    cv::solvePnP(objectPoints, imagePoints, A, distCoeffs, rvec, tvec);

    // extract rotation & translation matrix
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    cv::Mat R_inv = R.inv();
    _R = R;

    cv::Mat P = -R_inv*tvec;
    _t = tvec;
    double* p = (double *)P.data;

    qDebug() << "R = ";
    qDebug() << _R.at<double>(0,0) << _R.at<double>(0,1) << _R.at<double>(0,2) << _t.at<double>(0,0);
    qDebug() << _R.at<double>(1,0) << _R.at<double>(1,1) << _R.at<double>(1,2) << _t.at<double>(1,0);
    qDebug() << _R.at<double>(2,0) << _R.at<double>(2,1) << _R.at<double>(2,2) << _t.at<double>(2,0);

    // camera posiation
    //printf("x=%lf, y=%lf, z=%lf", p[0], p[1], p[2]);
    cv::destroyAllWindows();

    QSettings qsettings("GSCT", "Turtle");
    qsettings.setValue("BaseWidth",ui->numWidth->value());
    qsettings.setValue("BaseHeight",ui->numHeight->value());
    qsettings.setValue("CenterWidth",ui->numCenterWidth->value());
    qsettings.setValue("CenterHeight",ui->numCenterHeight->value());
}

void MainWindow::on_butStart_clicked()
{
    QSettings qsettings("GSCT", "Turtle");
    qsettings.setValue("CubeSize",ui->numCubeSize->value());

    cv::namedWindow("Play");
    double size = ui->numCubeSize->value();
    cv::Mat point = cv::Mat(3,1,CV_64FC1);
    point.at<double>(0,0) = size/2;
    point.at<double>(1,0) = size/2;
    point.at<double>(2,0) = size;
    cv::Mat intrinsic = _camera._intrinsic_param;
    while(1)
    {
        bool end_flag = false;
        //Calibration Update;
        cv::Mat temp_world_point = _R * point + _t;
        qDebug() << "1: " << point.at<double>(0,0) << point.at<double>(1,0) << point.at<double>(2,0);
        qDebug() << "2: " << temp_world_point.at<double>(0,0) << temp_world_point.at<double>(1,0) << temp_world_point.at<double>(2,0);
        cv::Mat UV_point = intrinsic * temp_world_point;
        UV_point = UV_point / UV_point.at<double>(2,0);
        qDebug() << "3: " << UV_point.at<double>(0,0) << UV_point.at<double>(1,0);
        _camera._mutex.lock();
        cv::Mat image = _camera._image.clone();
        _camera._mutex.unlock();
        cv::Point2f temp_point = cv::Point2f(UV_point.at<double>(0,0),UV_point.at<double>(1,0));
        //qDebug() << temp_point.x << temp_point.y;
        cv::circle(image,temp_point,10,cv::Scalar(0,0,255),2);
        //Turtle ->
        //cv::circle(image,_point[4],5,cv::Scalar(0,0,255),2);
        cv::imshow("Play",image);
        int ch = cv::waitKey(0);
        switch(ch)
        {
            case 'W':
            case 'w':
            case Qt::Key_Up:
                point.at<double>(0,0) -= size;
            break;
            case 'A':
            case 'a':
                point.at<double>(1,0) -= size;
            break;
            case 'S':
            case 's':
                point.at<double>(0,0) += size;
            break;
            case 'D':
            case 'd':
                point.at<double>(1,0) += size;
            break;
            case 'u':
            case 'U':
                point.at<double>(2,0) += size;
            break;
            case 'j':
            case 'J':
                point.at<double>(2,0) -= size;
            break;
            case 'e':
            case 'E':
                end_flag = true;
            break;
            case 'r':
            case 'R':
                point.at<double>(0,0) = size/2;
                point.at<double>(1,0) = size/2;
                point.at<double>(2,0) = size;
            break;
        }

        if(end_flag) break;
    }
    cv::destroyAllWindows();
}

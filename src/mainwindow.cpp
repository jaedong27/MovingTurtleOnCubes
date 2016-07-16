#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QSettings"
#include "QElapsedTimer"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _click_index = 0;
    _point[0] = cv::Point2i(10 ,10 );
    _point[1] = cv::Point2i(200,10 );
    _point[2] = cv::Point2i(200,200);
    _point[3] = cv::Point2i(10 ,200);
    _point[4] = cv::Point2i(100,100);

    QSettings qsettings("GSCT", "Turtle_");
    ui->numCameraIndex->setValue(qsettings.value("CameraIndex",0).toInt());
    ui->numCubeSize->setValue(qsettings.value("CubeSize",0).toFloat());

    _turtle_image = cv::imread("./turtle_.png");//,cv::IMREAD_UNCHANGED);
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

void MainWindow::drawTurtle(cv::Mat &image, cv::Point points[4])
{
    if(_turtle_image.empty()) return;

    int img_width = _turtle_image.cols;
    int img_height = _turtle_image.rows;

    //-- Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;

    obj.clear();
    scene.clear();

    obj.push_back(cv::Point2f(0,0));
    obj.push_back(cv::Point2f(img_width-1,0));
    obj.push_back(cv::Point2f(img_width-1,img_height-1));
    obj.push_back(cv::Point2f(0,img_height-1));

    for(int i = 0 ; i < 4 ; i++)
    {
        //qDebug() << points[i].x << points[i].y;
        cv::Point2f scene_point = points[i];
        scene.push_back(scene_point);
    }

    cv::Mat H = cv::findHomography(scene,obj);
    //qDebug() << "jaedong : " << H.type()<< H.rows << H.cols;

    int min_u = 99999;
    int max_u = -1;
    int min_v = 99999;
    int max_v = -1;

    for(int i = 0 ; i < 4 ; i++)
    {
        min_u = qMin<int>(min_u,obj[i].x);
        min_v = qMin<int>(min_v,obj[i].y);
        max_u = qMax<int>(max_u,obj[i].x);
        max_v = qMax<int>(max_v,obj[i].y);
    }

    min_u = qMax<int>(0, min_u);
    min_v = qMax<int>(0, min_v);
    max_u = qMax<int>(image.cols, max_u);
    max_v = qMax<int>(image.rows, max_v);
    cv::Rect rect = cv::Rect(cv::Point(0,0),cv::Point(img_width,img_height));
    cv::Rect image_rect = cv::Rect(cv::Point(0,0),cv::Point(image.cols,image.rows));
    for(int v = min_v ; v < max_v ; v++)
    {
        for(int u = min_u ; u < max_u ; u++)
        {
            //qDebug() << u << v;
            cv::Mat point = cv::Mat(3,1,CV_64F);
            point.at<double>(0,0) = u;
            point.at<double>(1,0) = v;
            point.at<double>(2,0) = 1;
            point = H * point;
            point = point / point.at<double>(2,0);
            cv::Point2f img_point = cv::Point2f(point.at<double>(0,0),point.at<double>(1,0));
            //cv::Point2f target_point = cv::Point2f(u,v);
            if(point.at<double>(0,0) > 0 && point.at<double>(1,0) > 0
                    && point.at<double>(0,0) < img_width && point.at<double>(1,0) < img_height )//&& image_rect.contains(target_point))
            {
                if( _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[0] != 0
                        && _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[1] != 0
                        && _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[2] != 0)
                {
                    image.at<cv::Vec3b>(v,u)[0] = _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[0];// * _turtle_image.at<cv::Vec4b>(img_point.y,img_point.x)[3] / 255.0;
                    image.at<cv::Vec3b>(v,u)[1] = _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[1];// * _turtle_image.at<cv::Vec4b>(img_point.y,img_point.x)[3] / 255.0;
                    image.at<cv::Vec3b>(v,u)[2] = _turtle_image.at<cv::Vec3b>(img_point.y,img_point.x)[2];// * _turtle_image.at<cv::Vec4b>(img_point.y,img_point.x)[3] / 255.0;
                }
            }
        }
    }
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
        cv::line(image,_point[1],_point[2],cv::Scalar(255,255,0),1);
        cv::line(image,_point[2],_point[3],cv::Scalar(255,255,0),1);
        cv::line(image,_point[3],_point[0],cv::Scalar(255,255,0),1);
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
            case 'E':
            case 'e':
                end_flag = true;
            break;
        }

        if(end_flag) break;
    }

    // matching pairs
    std::vector<cv::Point3f> objectPoints;	// 3d world coordinates
    std::vector<cv::Point2f> imagePoints;	// 2d image coordinates

    float object_width  = 2;//ui->numCubeSize->value();
    float object_height = 2;//ui->numCubeSize->value();

    objectPoints.clear();
    objectPoints.push_back(cv::Point3f(0,0,0));
    objectPoints.push_back(cv::Point3f(object_width,0,0));
    objectPoints.push_back(cv::Point3f(object_width,object_height,0));
    objectPoints.push_back(cv::Point3f(0,object_height,0));

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
}

void MainWindow::on_butStart_clicked()
{
    QSettings qsettings("GSCT", "Turtle");
    qsettings.setValue("CubeSize",ui->numCubeSize->value());

    cv::namedWindow("Play");
    double size = 2;//ui->numCubeSize->value();
    cv::Mat point = cv::Mat(3,1,CV_64FC1);
    point.at<double>(0,0) = size/2;
    point.at<double>(1,0) = size/2;
    point.at<double>(2,0) = 0;
    cv::Mat intrinsic = _camera._intrinsic_param;

    cv::Mat point_list[4];
    cv::Point uv_point_list[4];
    for(int i = 0 ; i < 4 ; i++)
    {
        point_list[i] = cv::Mat::ones(3,1,CV_64F);
    }

    int direction = 0;
    cv::Point direction_position[4];
    direction_position[0] = cv::Point(1 , 0);
    direction_position[1] = cv::Point(0 , 1);
    direction_position[2] = cv::Point(-1, 0);
    direction_position[3] = cv::Point(0 ,-1);

    cv::Point line_position[4];
    line_position[0] = cv::Point(-1 , -1);
    line_position[1] = cv::Point( 1 , -1);
    line_position[2] = cv::Point( 1 ,  1);
    line_position[3] = cv::Point(-1 ,  1);

    while(1)
    {
        QElapsedTimer timer;
        timer.start();
        bool end_flag = false;
        //Calibration Update;
        //cv::Mat temp_world_point = _R * point + _t;
        //qDebug() << "1: " << point.at<double>(0,0) << point.at<double>(1,0) << point.at<double>(2,0);
        //qDebug() << "2: " << temp_world_point.at<double>(0,0) << temp_world_point.at<double>(1,0) << temp_world_point.at<double>(2,0);
        //cv::Mat UV_point = intrinsic * temp_world_point;
        //UV_point = UV_point / UV_point.at<double>(2,0);
        //qDebug() << "3: " << UV_point.at<double>(0,0) << UV_point.at<double>(1,0);
        _camera._mutex.lock();
        cv::Mat image = _camera._image.clone();
        _camera._mutex.unlock();

        //cv::Point2f temp_point = cv::Point2f(UV_point.at<double>(0,0),UV_point.at<double>(1,0));
        //qDebug() << temp_point.x << temp_point.y;
        qDebug() << "1 : " << timer.elapsed();
        for(int i = 0 ; i < 4; i++)
        {
            point_list[i].at<double>(0,0) = point.at<double>(0,0) + line_position[(direction+i)%4].x;
            point_list[i].at<double>(1,0) = point.at<double>(1,0) + line_position[(direction+i)%4].y;
            point_list[i].at<double>(2,0) = point.at<double>(2,0);
            cv::Mat temp = _R * point_list[i] + _t;
            temp = intrinsic * temp;
            temp = temp / temp.at<double>(2,0);
            uv_point_list[i] = cv::Point(temp.at<double>(0,0),temp.at<double>(1,0));
        }

//        point_list[0].at<double>(0,0) = 0;    point_list[0].at<double>(1,0) = 0;
//        point_list[1].at<double>(0,0) = size; point_list[1].at<double>(1,0) = 0;
//        point_list[2].at<double>(0,0) = 0;    point_list[2].at<double>(1,0) = size;
//        point_list[3].at<double>(0,0) = size; point_list[3].at<double>(1,0) = size;
        qDebug() << "2 : " << timer.elapsed();
        drawTurtle(image,uv_point_list);
        cv::circle(image,uv_point_list[0],3,cv::Scalar(0,0,255),-1);
        cv::circle(image,uv_point_list[1],3,cv::Scalar(255,0,0),-1);
        cv::circle(image,uv_point_list[2],3,cv::Scalar(255,255,255),-1);
        cv::circle(image,uv_point_list[3],3,cv::Scalar(0,255,255),-1);
        qDebug() << "3 : " << timer.elapsed();

        //Turtle ->

        qDebug() << "4 : " << timer.elapsed();
        //cv::circle(image,_point[4],5,cv::Scalar(0,0,255),2);
        cv::imshow("Play",image);
        qDebug() << "5 : " << timer.elapsed();
        int ch = cv::waitKey(30);
        qDebug() << "6 : " << timer.elapsed();
        switch(ch)
        {
            case 'W':
            case 'w':
            case Qt::Key_Up:
                point.at<double>(1,0) -= size;
            break;
            case 'A':
            case 'a':
                point.at<double>(0,0) -= size;
            break;
            case 'S':
            case 's':
                point.at<double>(1,0) += size;
            break;
            case 'D':
            case 'd':
                point.at<double>(0,0) += size;
            break;
            case 'u':
            case 'U':
                point.at<double>(2,0) -= size;
            break;
            case 'j':
            case 'J':
                point.at<double>(2,0) += size;
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

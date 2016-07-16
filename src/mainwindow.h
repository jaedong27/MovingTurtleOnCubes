#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <Camera.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Camera _camera;
    cv::Point2i _point[5];
    int _click_index;
    cv::Mat _R,_t;
    cv::Mat _turtle_image;

    static void onMouse( int evt, int x, int y, int flags, void* param );

    void drawTurtle(cv::Mat &image, cv::Point points[4]);

private slots:
    void on_butCalibration_clicked();

    void on_butCameraOpen_clicked();

    void on_butStart_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

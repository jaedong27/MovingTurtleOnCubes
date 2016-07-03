#-------------------------------------------------
#
# Project created by QtCreator 2016-07-02T13:08:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MovingTurtleOnCubes
TEMPLATE = app

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

#DEFINES += NOMINMAX

macx{
    INCLUDEPATH += "/usr/local/Cellar/opencv3/3.1.0_3/include"
    LIBS += -L"/usr/local/Cellar/opencv3/3.1.0_3/lib"
    LIBS += -lopencv_core
    LIBS += -lopencv_imgcodecs
    LIBS += -lopencv_highgui
    LIBS += -lopencv_imgproc
    LIBS += -lopencv_video
    LIBS += -lopencv_videoio
    LIBS += -lopencv_calib3d
}

SOURCES += main.cpp\
        mainwindow.cpp \
    Camera.cpp

HEADERS  += mainwindow.h \
    Camera.h

FORMS    += mainwindow.ui

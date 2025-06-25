#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>

#include "usermanagewin.h"

namespace Ui
{
    class mainwin;
}

class mainwin : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwin(QWidget *parent = nullptr);
    ~mainwin();

private slots:
    void on_userManageButton_clicked();
    void on_openCameraButton_clicked();
    void on_closeCameraButton_clicked();
    void on_snapButton_clicked();

private:
    Ui::mainwin *ui;

    usermanagewin *userManageWin; // 用户管理窗口

    QCamera *camera = nullptr;                   // 摄像头
    QCameraViewfinder *viewfinder = nullptr;     // 取景器
    QCameraImageCapture *imageCapture = nullptr; // 抓拍
};

#endif // MAINWIN_H

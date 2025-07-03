#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QTcpServer>
#include <QTcpSocket>

#include "usermanagewin.h"

namespace Ui
{
    class mainwin;
}

class TcpServer;
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

    // TCP服务端
    TcpServer *tcpServer = nullptr;

#if defined(Q_OS_WIN)
    QCamera *camera = nullptr;                   // 摄像头
    QCameraViewfinder *viewfinder = nullptr;     // 取景器
    QCameraImageCapture *imageCapture = nullptr; // 抓拍
#elif defined(Q_OS_LINUX)
#include <QTimer>
#include <QLabel> // 添加 QLabel 头文件
#define V4L2_BUFFER_COUNT 4
    int v4l2_fd = -1;
    QTimer *v4l2_timer = nullptr;
    QLabel *cameraDisplayLabel = nullptr; // 用于显示摄像头画面的 QLabel
    struct
    {
        void *start;
        size_t length;
    } v4l2_buffers[V4L2_BUFFER_COUNT];
#endif

private slots:
    void onTcpDataReceived(QTcpSocket *client, const QByteArray &data);
    void on_uploadTHDataBtn_clicked();
};

#endif // MAINWIN_H

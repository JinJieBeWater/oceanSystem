#include "mainwin.h"
#include "ui_mainwin.h"
#include <QVBoxLayout>
#include <QCameraInfo>

mainwin::mainwin(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::mainwin)
{
    ui->setupUi(this);
    userManageWin = new usermanagewin(this);

    // 摄像头相关初始化
    viewfinder = new QCameraViewfinder(this);
    viewfinder->setMaximumHeight(200); // 限制摄像头显示区最大高度
    QVBoxLayout *layout = new QVBoxLayout(ui->cameraDisplay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(viewfinder);
    ui->cameraDisplay->setLayout(layout);

    // 枚举摄像头并填充下拉框
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras)
    {
        ui->cameraComboBox->addItem(cameraInfo.description(), cameraInfo.deviceName());
    }

    // 抓拍相关初始化
    imageCapture = new QCameraImageCapture(nullptr, this);
}

mainwin::~mainwin()
{
    if (camera)
    {
        camera->stop();
        delete camera;
    }
    delete ui;
}

void mainwin::on_userManageButton_clicked()
{
    userManageWin->show();
    this->hide(); // 隐藏当前窗口
}

void mainwin::on_openCameraButton_clicked()
{
    // 每次都重建摄像头对象，确保可以切换摄像头
    if (camera)
    {
        camera->stop();
        delete camera;
        camera = nullptr;
    }
    int index = ui->cameraComboBox->currentIndex();
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty() || index < 0 || index >= cameras.size())
    {
        // 可根据需要弹窗提示无摄像头
        return;
    }
    camera = new QCamera(cameras.at(index), this);
    camera->setViewfinder(viewfinder);
    if (imageCapture)
    {
        // 断开旧的 imageCaptured 信号，防止重复
        disconnect(imageCapture, nullptr, this, nullptr);
        delete imageCapture;
    }
    imageCapture = new QCameraImageCapture(camera, this);
    connect(imageCapture, &QCameraImageCapture::imageCaptured, this, [this](int, const QImage &image)
            {
        QLabel *imgLabel = new QLabel(ui->snapShotArea);
        QSize thumbSize(200, 150); // 统一缩略图尺寸
        imgLabel->setPixmap(QPixmap::fromImage(image).scaled(thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QVBoxLayout *snapLayout = qobject_cast<QVBoxLayout*>(ui->snapShotArea->layout());
        if (snapLayout) {
            snapLayout->addWidget(imgLabel);
        } });
    camera->start();
}

void mainwin::on_snapButton_clicked()
{
    if (camera && imageCapture && camera->state() == QCamera::ActiveState)
    {
        imageCapture->capture();
    }
}

void mainwin::on_closeCameraButton_clicked()
{
    if (camera)
    {
        camera->stop();
    }
}

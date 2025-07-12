#define CAMERA_WIDTH 800
#define CAMERA_HEIGHT 480
#define CAMERA_SET_WIDTH 800
#define CAMERA_SET_HEIGHT 480
#define TCP_SERVER_PORT 6666
#define LOCAL_HOST_IP "192.168.14.49"

#include <QDebug>
#include "mainwin.h"
#include "libs/tcpserver.h"
#include "ui_mainwin.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QMap>
#include <QScrollBar>

#if defined(Q_OS_WIN)
#include <QCameraInfo>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#elif defined(Q_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <QImage>
#include <QTimer>
#include <cstring>
#include <sys/mman.h>
#include "yuvutils.h" // 包含yuvutils.h头文件
#endif

mainwin::mainwin(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::mainwin)
{
    ui->setupUi(this);
    userManageWin = new usermanagewin(this);

    // 初始化TCP服务端对象，但不自动启动，受控于按钮
    tcpServer = new TcpServer(this);
    connect(tcpServer, &TcpServer::dataReceived, this, &mainwin::onTcpDataReceived);
    connect(tcpServer, &TcpServer::newClientConnected, this, &mainwin::onNewClientConnected);
    connect(tcpServer, &TcpServer::clientDisconnected, this, &mainwin::onClientDisconnected);
    // 不自动启动，等待用户点击 startServerBtn
    // 设置默认的TCP服务端ip
    if (ui->serverIpEdit)
        ui->serverIpEdit->setText(LOCAL_HOST_IP);
    // 可在UI初始化时设置默认端口
    if (ui->serverPortEdit)
        ui->serverPortEdit->setText(QString::number(TCP_SERVER_PORT));

#if defined(Q_OS_WIN)
    // Windows 摄像头相关初始化
    viewfinder = new QCameraViewfinder(this);
    QVBoxLayout *layout = new QVBoxLayout(ui->cameraDisplay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(viewfinder);
    ui->cameraDisplay->setLayout(layout);

    // 确保 snapShotArea 有 QVBoxLayout
    if (!ui->snapShotArea->layout())
    {
        QVBoxLayout *snapLayout = new QVBoxLayout(ui->snapShotArea);
        snapLayout->setContentsMargins(0, 0, 0, 0);
        ui->snapShotArea->setLayout(snapLayout);
    }
    else
    {
        // 检查类型，不是 QVBoxLayout 也替换
        QVBoxLayout *testLayout = qobject_cast<QVBoxLayout *>(ui->snapShotArea->layout());
        if (!testLayout)
        {
            delete ui->snapShotArea->layout();
            QVBoxLayout *snapLayout = new QVBoxLayout(ui->snapShotArea);
            snapLayout->setContentsMargins(0, 0, 0, 0);
            ui->snapShotArea->setLayout(snapLayout);
        }
    }

    // 枚举摄像头并填充下拉框
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &cameraInfo : cameras)
    {
        ui->cameraComboBox->addItem(cameraInfo.description(), cameraInfo.deviceName());
    }

    // 抓拍相关初始化
    imageCapture = new QCameraImageCapture(nullptr, this);
#elif defined(Q_OS_LINUX)
    // v4l2相关成员变量初始化
    v4l2_fd = -1;         // 摄像头设备文件描述符
    v4l2_timer = nullptr; // 定时器用于定时采集帧
    for (int i = 0; i < V4L2_BUFFER_COUNT; ++i)
    {
        v4l2_buffers[i].start = nullptr;
        v4l2_buffers[i].length = 0;
    }
    // Linux 摄像头显示初始化
    cameraDisplayLabel = new QLabel(this);
    cameraDisplayLabel->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout = new QVBoxLayout(ui->cameraDisplay);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(cameraDisplayLabel);
    ui->cameraDisplay->setLayout(layout);
    // 隐藏摄像头下拉框
    ui->cameraComboBox->setVisible(false);
#endif
}

// 服务端启动按钮槽函数
void mainwin::on_startServerBtn_clicked()
{
    if (!tcpServer)
        return;
    QString ip = ui->serverIpEdit ? ui->serverIpEdit->text().trimmed() : LOCAL_HOST_IP;
    quint16 port = ui->serverPortEdit ? ui->serverPortEdit->text().toUShort() : TCP_SERVER_PORT;
    if (tcpServer->isListening())
    {
        QMessageBox::information(this, "提示", "服务端已在监听中");
        return;
    }
    if (!tcpServer->startServer(port, ip))
    {
        QMessageBox::warning(this, "TCP服务端启动失败", QString("无法监听 %1:%2").arg(ip).arg(port));
    }
    else
    {
        statusBar()->showMessage(QString("TCP服务端已启动，监听 %1:%2").arg(ip).arg(port), 3000);
        QMessageBox::information(this, "启动成功", QString("服务端已启动: %1:%2").arg(ip).arg(port));
    }
}

// 服务端关闭按钮槽函数
void mainwin::on_closeServerBtn_clicked()
{
    if (!tcpServer)
        return;
    if (!tcpServer->isListening())
    {
        QMessageBox::information(this, "提示", "服务端未在监听");
        return;
    }
    tcpServer->stopServer();
    statusBar()->showMessage("服务端已关闭", 3000);
    QMessageBox::information(this, "关闭成功", "服务端已关闭");
    // 清空所有客户端
    for (auto it = clientMap.begin(); it != clientMap.end(); ++it)
    {
        if (it.value())
        {
            it.value()->disconnectFromHost();
            it.value()->deleteLater();
        }
    }
    clientMap.clear();
    ui->socketsComboBox->clear();
}

mainwin::~mainwin()
{
#if defined(Q_OS_WIN)
    if (camera)
    {
        camera->stop();
        delete camera;
    }
#elif defined(Q_OS_LINUX)
    // 释放v4l2相关资源
    if (v4l2_timer)
        v4l2_timer->stop();
    if (v4l2_fd >= 0)
    {
        // 关闭流
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type);
        // 释放缓冲区映射
        for (int i = 0; i < V4L2_BUFFER_COUNT; ++i)
        {
            if (v4l2_buffers[i].start)
                munmap(v4l2_buffers[i].start, v4l2_buffers[i].length);
            v4l2_buffers[i].start = nullptr;
            v4l2_buffers[i].length = 0;
        }
        close(v4l2_fd);
        v4l2_fd = -1;
    }
    if (cameraDisplayLabel)
    {
        delete cameraDisplayLabel;
        cameraDisplayLabel = nullptr;
    }
#endif
    if (tcpServer)
    {
        tcpServer->stopServer();
    }
    delete ui;
}

// TCP数据接收槽函数
void mainwin::onTcpDataReceived(QTcpSocket *client, const QByteArray &data)
{
    // 假设数据格式为 "T:23.5;H:56.7"，T为温度，H为湿度
    QString str = QString::fromUtf8(data).trimmed();
    // 显示到命令显示区
    if (ui->commandShowArea)
    {
        QString from = client ? client->peerAddress().toString() : "未知";
        if (from.startsWith("::ffff:"))
            from = from.mid(7);
        QString oldText = ui->commandShowArea->toPlainText();
        QString newText = oldText + QString("[来自%1:%2] %3\n").arg(from).arg(client ? client->peerPort() : 0).arg(str);
        ui->commandShowArea->setPlainText(newText);
        ui->commandShowArea->verticalScrollBar()->setValue(ui->commandShowArea->verticalScrollBar()->maximum());
    }
    QRegExp rx("T:([0-9.]+);H:([0-9.]+)");
    if (rx.indexIn(str) != -1)
    {
        QString temp = rx.cap(1);
        QString hum = rx.cap(2);
        ui->temperatureShow->setText(temp);
        ui->humidityShow->setText(hum);
        statusBar()->showMessage(QString("收到温湿度: %1°C, %2%RH").arg(temp, hum), 3000);
    }
}

// 新增：客户端连接/断开处理
void mainwin::onNewClientConnected(QTcpSocket *client)
{
    QString ip = client->peerAddress().toString();
    if (ip.startsWith("::ffff:"))
        ip = ip.mid(7);
    qDebug() << ip << ":" << client->peerPort();
    QString key = QString("%1:%2").arg(ip).arg(client->peerPort());
    clientMap[key] = client;
    ui->socketsComboBox->addItem(key);
}

void mainwin::onClientDisconnected(QTcpSocket *client)
{
    QString ip = client->peerAddress().toString();
    if (ip.startsWith("::ffff:"))
        ip = ip.mid(7);
    QString key = QString("%1:%2").arg(ip).arg(client->peerPort());
    clientMap.remove(key);
    int idx = ui->socketsComboBox->findText(key);
    if (idx >= 0)
        ui->socketsComboBox->removeItem(idx);
}

void mainwin::on_userManageButton_clicked()
{
    userManageWin->show();
    this->hide(); // 隐藏当前窗口
}

#if defined(Q_OS_WIN)
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
    qDebug() << "[on_openCameraButton_clicked] cameras count:" << cameras.size() << ", index:" << index;
    if (cameras.isEmpty() || index < 0 || index >= cameras.size())
    {
        qDebug() << "[on_openCameraButton_clicked] No camera found or invalid index.";
        // 可根据需要弹窗提示无摄像头
        return;
    }
    camera = new QCamera(cameras.at(index), this);
    qDebug() << "[on_openCameraButton_clicked] Created QCamera for device:" << cameras.at(index).deviceName();
    camera->setViewfinder(viewfinder);
    if (imageCapture)
    {
        // 断开旧的 imageCaptured 信号，防止重复
        disconnect(imageCapture, nullptr, this, nullptr);
        delete imageCapture;
        qDebug() << "[on_openCameraButton_clicked] Deleted old imageCapture.";
    }
    imageCapture = new QCameraImageCapture(camera, this);
    qDebug() << "[on_openCameraButton_clicked] Created new QCameraImageCapture.";
    connect(imageCapture, &QCameraImageCapture::imageCaptured, this, [this](int id, const QImage &image)
            {
        qDebug() << "[imageCaptured] id:" << id << ", image size:" << image.size();
        QLabel *imgLabel = new QLabel(ui->snapShotArea);
        QSize thumbSize(160, 120); // 统一缩略图尺寸
        imgLabel->setPixmap(QPixmap::fromImage(image).scaled(thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QVBoxLayout *snapLayout = qobject_cast<QVBoxLayout*>(ui->snapShotArea->layout());
        if (snapLayout) {
            snapLayout->addWidget(imgLabel);
            qDebug() << "[imageCaptured] Added snapshot to snapShotArea.";
        } });
    camera->start();
}

void mainwin::on_snapButton_clicked()
{
    if (!camera || !imageCapture)
    {
        QMessageBox::warning(this, "Error", "Camera not initialized");
        return;
    }

    if (camera->state() != QCamera::ActiveState)
    {
        QMessageBox::warning(this, "Error", "Camera is not active");
        return;
    }

    // 连接错误信号
    connect(imageCapture, QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
            this, [this](int id, QCameraImageCapture::Error /*error*/, const QString &errorString)
            {
                Q_UNUSED(id);
                QMessageBox::warning(this, "Capture Error",
                    QString("Failed to capture image: %1").arg(errorString)); });

    // 连接捕获完成信号
    connect(imageCapture, &QCameraImageCapture::imageSaved, this, [this](int id, const QString &fileName)
            {
        Q_UNUSED(id);
        statusBar()->showMessage(QString("Image saved to %1").arg(fileName), 3000); });

    imageCapture->capture();
}

void mainwin::on_closeCameraButton_clicked()
{
    if (camera)
    {
        camera->stop();
    }
}
#elif defined(Q_OS_LINUX)
// v4l2 摄像头相关槽函数实现（标准流程）
void mainwin::on_openCameraButton_clicked()
{
    if (v4l2_fd >= 0)
        return; // 已打开
    v4l2_fd = open("/dev/video7", O_RDWR);
    if (v4l2_fd < 0)
    {
        // 打开失败，可能是设备不存在或权限不足
        QMessageBox::warning(this, "Error", "Failed to open camera device.");
        return;
    }

    // 1. 设置采集格式
    struct v4l2_format vfmt;
    memset(&vfmt, 0, sizeof(vfmt));
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vfmt.fmt.pix.width = CAMERA_SET_WIDTH;
    vfmt.fmt.pix.height = CAMERA_SET_HEIGHT;
    vfmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    vfmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (ioctl(v4l2_fd, VIDIOC_S_FMT, &vfmt) < 0)
    {
        QString errMsg = QString("VIDIOC_S_FMT failed: %1").arg(strerror(errno));
        qDebug::error(errMsg.toStdString());
        QMessageBox::warning(this, "ioctl error", errMsg);
    }

    // 2. 申请缓冲区
    v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = V4L2_BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(v4l2_fd, VIDIOC_REQBUFS, &req) < 0)
    {
        QString errMsg = QString("VIDIOC_REQBUFS failed: %1").arg(strerror(errno));
        qDebug::error(errMsg.toStdString());
        QMessageBox::warning(this, "ioctl error", errMsg);
    }

    // 3. 查询缓冲区并mmap映射
    for (int i = 0; i < V4L2_BUFFER_COUNT; ++i)
    {
        v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(v4l2_fd, VIDIOC_QUERYBUF, &buf) < 0)
        {
            QString errMsg = QString("VIDIOC_QUERYBUF failed: %1").arg(strerror(errno));
            qDebug::error(errMsg.toStdString());
            QMessageBox::warning(this, "ioctl error", errMsg);
        }
        v4l2_buffers[i].length = buf.length;
        v4l2_buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_fd, buf.m.offset);
    }

    // 4. 入队所有缓冲区
    for (int i = 0; i < V4L2_BUFFER_COUNT; ++i)
    {
        v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(v4l2_fd, VIDIOC_QBUF, &buf) < 0)
        {
            QString errMsg = QString("VIDIOC_QBUF failed: %1").arg(strerror(errno));
            qDebug::error(errMsg.toStdString());
            QMessageBox::warning(this, "ioctl error", errMsg);
        }
    }

    // 5. 启动视频流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(v4l2_fd, VIDIOC_STREAMON, &type) < 0)
    {
        QString errMsg = QString("VIDIOC_STREAMON failed: %1").arg(strerror(errno));
        qDebug::error(errMsg.toStdString());
        QMessageBox::warning(this, "ioctl error", errMsg);
    }

    // 6. 启动定时器循环采集
    if (!v4l2_timer)
    {
        v4l2_timer = new QTimer(this);
        connect(v4l2_timer, &QTimer::timeout, this, [this]()
                {
            // 7. 出队一帧
            v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            ioctl(v4l2_fd, VIDIOC_DQBUF, &buf);
            
            // 8. YUYV转RGB并显示
            unsigned char *yuyv = (unsigned char*)v4l2_buffers[buf.index].start;
            QImage img(CAMERA_WIDTH, CAMERA_HEIGHT, QImage::Format_RGB888);
            for (int i = 0, j = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT * 2; i += 4, j += 2) {
                int y0 = yuyv[i], u = yuyv[i+1], y1 = yuyv[i+2], v = yuyv[i+3];
                int px = (j % CAMERA_WIDTH), py = (j / CAMERA_WIDTH);
                img.setPixel(px, py, yuyv2rgb(y0, u, v));
                img.setPixel(px+1, py, yuyv2rgb(y1, u, v));
            }
            // 更新 cameraDisplayLabel 的 Pixmap
            cameraDisplayLabel->setPixmap(QPixmap::fromImage(img).scaled(cameraDisplayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            // 9. 再次入队
            if (ioctl(v4l2_fd, VIDIOC_QBUF, &buf) < 0) {
                QString errMsg = QString("VIDIOC_QBUF failed: %1").arg(strerror(errno));
                qDebug::error(errMsg.toStdString());
                QMessageBox::warning(this, "ioctl error", errMsg);
            } });
    }
    v4l2_timer->start(66); // 15fps
}

void mainwin::on_snapButton_clicked()
{
    // 单帧采集：出队、显示、再入队
    qDebug << "Snap button clicked, capturing a single frame." << std::endl;
    if (v4l2_fd < 0)
        return;
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(v4l2_fd, VIDIOC_DQBUF, &buf) < 0)
    {
        QString errMsg = QString("VIDIOC_DQBUF failed: %1").arg(strerror(errno));
        qDebug::error(errMsg.toStdString());
        QMessageBox::warning(this, "ioctl error", errMsg);
        return;
    }
    unsigned char *yuyv = (unsigned char *)v4l2_buffers[buf.index].start;
    QImage img(CAMERA_WIDTH, CAMERA_HEIGHT, QImage::Format_RGB888);
    for (int i = 0, j = 0; i < CAMERA_WIDTH * CAMERA_HEIGHT * 2; i += 4, j += 2)
    {
        int y0 = yuyv[i], u = yuyv[i + 1], y1 = yuyv[i + 2], v = yuyv[i + 3];
        int px = (j % CAMERA_WIDTH), py = (j / CAMERA_WIDTH);
        img.setPixel(px, py, yuyv2rgb(y0, u, v));
        img.setPixel(px + 1, py, yuyv2rgb(y1, u, v));
    }
    // 将抓拍的图像显示到 snapShotArea
    QLabel *imgLabel = new QLabel(ui->snapShotArea);
    QSize thumbSize(160, 120);
    imgLabel->setPixmap(QPixmap::fromImage(img).scaled(thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QVBoxLayout *snapLayout = qobject_cast<QVBoxLayout *>(ui->snapShotArea->layout());
    if (snapLayout)
        snapLayout->addWidget(imgLabel);
    if (ioctl(v4l2_fd, VIDIOC_QBUF, &buf) < 0)
    {
        QString errMsg = QString("VIDIOC_QBUF failed: %1").arg(strerror(errno));
        qDebug::error(errMsg.toStdString());
        QMessageBox::warning(this, "ioctl error", errMsg);
    }
}

void mainwin::on_closeCameraButton_clicked()
{
    // 关闭摄像头并释放资源
    if (v4l2_timer)
        v4l2_timer->stop();
    if (v4l2_fd >= 0)
    {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type) < 0)
        {
            QString errMsg = QString("VIDIOC_STREAMOFF failed: %1").arg(strerror(errno));
            qDebug::error(errMsg.toStdString());
            QMessageBox::warning(this, "ioctl error", errMsg);
        }
        for (int i = 0; i < V4L2_BUFFER_COUNT; ++i)
        {
            if (v4l2_buffers[i].start)
                munmap(v4l2_buffers[i].start, v4l2_buffers[i].length);
            v4l2_buffers[i].start = nullptr;
            v4l2_buffers[i].length = 0;
        }
        close(v4l2_fd);
        v4l2_fd = -1;
    }
}
#endif

void mainwin::on_uploadTHDataBtn_clicked()
{
    // 创建客户端socket
    QTcpSocket *client = new QTcpSocket(this);
    client->connectToHost(LOCAL_HOST_IP, TCP_SERVER_PORT);
    if (!client->waitForConnected(1000))
    {
        QMessageBox::warning(this, "连接失败", QString("无法连接到本地TCP服务端: %1").arg(TCP_SERVER_PORT));
        client->deleteLater();
        return;
    }
    // 随机生成温湿度数据
    qsrand(static_cast<uint>(QTime::currentTime().msec() + QTime::currentTime().second() * 1000));
    double tempVal = 20.0 + (qrand() % 1500) / 100.0; // 20.00~34.99
    double humVal = 30.0 + (qrand() % 7000) / 100.0;  // 30.00~99.99
    QString temp = QString::number(tempVal, 'f', 2);
    QString hum = QString::number(humVal, 'f', 2);
    QString msg = QString("T:%1;H:%2").arg(temp, hum);
    client->write(msg.toUtf8());
    client->flush();
    client->waitForBytesWritten(500);
    client->disconnectFromHost();
    client->deleteLater();
    statusBar()->showMessage("随机温湿度数据已上传: " + msg, 3000);
}

void mainwin::on_addConnectBtn_clicked()
{
    // 获取用户输入的IP和端口（假设有对应的输入框）
    QString ip = ui->ipAddEdit ? ui->ipAddEdit->text() : LOCAL_HOST_IP;
    quint16 port = ui->portAddEdit ? ui->portAddEdit->text().toUShort() : TCP_SERVER_PORT;
    QString key = QString("%1:%2").arg(ip).arg(port);
    if (clientMap.contains(key) && clientMap[key] && clientMap[key]->state() == QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this, "已连接", QString("已与服务器 %1 保持长连接").arg(key));
        return;
    }
    // 若已存在但断开，先清理
    if (clientMap.contains(key) && clientMap[key])
    {
        clientMap[key]->abort();
        clientMap[key]->deleteLater();
        clientMap.remove(key);
    }
    QTcpSocket *client = new QTcpSocket(this);
    client->connectToHost(ip, port);
    if (!client->waitForConnected(1000))
    {
        QMessageBox::warning(this, "连接失败", QString("无法连接到服务器: %1").arg(key));
        client->deleteLater();
        return;
    }
    clientMap[key] = client;
    if (ui->socketsComboBox->findText(key) < 0)
        ui->socketsComboBox->addItem(key);
    statusBar()->showMessage(QString("成功建立长连接: %1").arg(key), 3000);
    QMessageBox::information(this, "连接成功", QString("已成功建立长连接: %1").arg(key));
    // 可在此处 connect client 的 readyRead 信号等，做后续通信
}

void mainwin::on_sendCommandBtn_clicked()
{
    // 获取选中的客户端
    QString key = ui->socketsComboBox->currentText();
    if (!clientMap.contains(key))
    {
        QMessageBox::warning(this, "错误", "未选择有效客户端");
        return;
    }
    QTcpSocket *client = clientMap[key];
    if (!client || client->state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::warning(this, "错误", "客户端已断开连接");
        return;
    }
    QString cmd = ui->sendCommandTextEdit->toPlainText();
    if (cmd.isEmpty())
    {
        QMessageBox::warning(this, "错误", "命令内容为空");
        return;
    }
    client->write(cmd.toUtf8());
    client->flush();
    statusBar()->showMessage("命令已发送: " + cmd, 3000);
}

void mainwin::on_closeConnectionBtn_clicked()
{
    // 获取当前选中的客户端 key
    QString key = ui->socketsComboBox->currentText();
    if (!clientMap.contains(key))
    {
        QMessageBox::warning(this, "错误", "未选择有效客户端");
        return;
    }
    QTcpSocket *client = clientMap[key];
    if (!client)
    {
        QMessageBox::warning(this, "错误", "客户端对象无效");
        clientMap.remove(key);
        int idx = ui->socketsComboBox->findText(key);
        if (idx >= 0)
            ui->socketsComboBox->removeItem(idx);
        return;
    }
    // 主动断开连接
    client->disconnectFromHost();
    if (client->state() != QAbstractSocket::UnconnectedState)
    {
        client->waitForDisconnected(1000);
    }
    client->deleteLater();
    clientMap.remove(key);
    int idx = ui->socketsComboBox->findText(key);
    if (idx >= 0)
        ui->socketsComboBox->removeItem(idx);
    statusBar()->showMessage("已断开连接: " + key, 3000);
}

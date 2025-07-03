#include "login.h"
#include <QApplication>
#include <QFileInfo>
#include <QDebug>

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QString prefix = QString("[oceanSystem][%1:%2 %3] ")
                         .arg(QFileInfo(context.file).fileName())
                         .arg(context.line)
                         .arg(context.function);
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "%sDebug: %s\n", prefix.toUtf8().constData(), localMsg.constData());
        fflush(stderr);
        break;
    case QtInfoMsg:
        fprintf(stderr, "%sInfo: %s\n", prefix.toUtf8().constData(), localMsg.constData());
        fflush(stderr);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%sWarning: %s\n", prefix.toUtf8().constData(), localMsg.constData());
        fflush(stderr);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%sCritical: %s\n", prefix.toUtf8().constData(), localMsg.constData());
        fflush(stderr);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%sFatal: %s\n", prefix.toUtf8().constData(), localMsg.constData());
        fflush(stderr);
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageHandler);
    QApplication a(argc, argv);
    login w;
    w.show();
    qDebug() << "Application started";
    return a.exec();
}

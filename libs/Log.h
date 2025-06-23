#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QFileInfo>

#define Log(...) qDebug(__VA_ARGS__).noquote().nospace() << "[" << QFileInfo(__FILE__).fileName() << ":" << __LINE__ << "]: "

#endif // LOG_H

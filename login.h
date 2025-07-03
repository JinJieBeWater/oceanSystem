#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QTextToSpeech>
#include <QSqlDatabase>

#include <QDebug>
#include "mainwin.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class login;
}
QT_END_NAMESPACE

class login : public QMainWindow
{
    Q_OBJECT

public:
    login(QWidget *parent = nullptr);
    ~login();

    void initializeDatabase();

    void createTables();

private slots:
    void on_signInBtn_clicked();

    void on_signUpBtn_clicked();

private:
    Ui::login *ui;
    QTextToSpeech *speech;
    mainwin *mainWin;
    QSqlDatabase db;
};

#endif // LOGIN_H

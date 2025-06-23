#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QTextToSpeech>

#include "mainwin.h"
#include "Log.h"

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

private slots:
    void on_signInBtn_clicked();

    void on_signUpBtn_clicked();

private:
    Ui::login *ui;
    QTextToSpeech *speech;
    mainwin *mainWin;
};
#endif // LOGIN_H

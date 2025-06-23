#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>

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

    void on_signUpBtn_pressed();

    void on_signUpBtn_released();

private:
    Ui::login *ui;
};
#endif // LOGIN_H

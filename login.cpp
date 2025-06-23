#include "login.h"
#include "ui_login.h"
#include <QDebug>

login::login(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::on_signInBtn_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    qDebug() << "Username:" << username;
    qDebug() << "Password:" << password;
    qDebug() << "Login button clicked";
}

void login::on_signUpBtn_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    qDebug() << "Username:" << username;
    qDebug() << "Password:" << password;
    qDebug() << "Sign Up button clicked";
}

void login::on_signUpBtn_pressed()
{
    // 设置背景颜色为红色
    this->setStyleSheet("QMainWindow{background-color: red;};");
}

void login::on_signUpBtn_released()
{
    // 设置背景颜色为蓝色
    this->setStyleSheet("QMainWindow{background-color: blue;};");
}

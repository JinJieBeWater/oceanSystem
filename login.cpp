#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

#include "mainwin.h"
#include "Log.h"

login::login(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::login)
{
    ui->setupUi(this);
    speech = new QTextToSpeech(this);
    mainWin = new mainwin(this);
}

login::~login()
{
    delete ui;
}

void login::on_signInBtn_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    Log() << "Username:" << username;
    Log() << "Password:" << password;

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
    }
    else
    {
        speech->say("Sign In successful!");
        mainWin->show();
        this->hide();
    }
}

void login::on_signUpBtn_clicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    Log() << "Username:" << username;
    Log() << "Password:" << password;

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
    }
    else
    {
        speech->say("Sign Up successful!");
        mainWin->show();
        this->hide();
    }
}

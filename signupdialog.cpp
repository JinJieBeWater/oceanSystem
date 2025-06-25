#include "signupdialog.h"
#include "ui_signupdialog.h"

signupdialog::signupdialog(QWidget *parent) : QDialog(parent),
                                              ui(new Ui::signupdialog)
{
    ui->setupUi(this);
    // 数据库连接
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("D:/University/Yue inlay/oceanSystem.db");
        db.open();
    }
}

signupdialog::~signupdialog()
{
    delete ui;
}

void signupdialog::onSignUpClicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordlineEdit->text().trimmed();
    // 输入验证
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
        return;
    }
    if (username.length() < 3 || password.length() < 6)
    {
        QMessageBox::critical(this, "Error", "Username must be at least 3 characters and password at least 6 characters long!");
        return;
    }
    if (username.length() > 20 || password.length() > 20)
    {
        QMessageBox::critical(this, "Error", "Username and password cannot exceed 20 characters!");
        return;
    }
    if (username.contains(" ") || password.contains(" "))
    {
        QMessageBox::critical(this, "Error", "Username and password cannot contain spaces!");
        return;
    }
    if (username.contains(";") || password.contains(";"))
    {
        QMessageBox::critical(this, "Error", "Username and password cannot contain semicolons!");
        return;
    }
    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Database Error", "Database is not open!");
        return;
    }
    // 检查用户名是否已存在
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id FROM users WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec())
    {
        QMessageBox::critical(this, "Database Error", "Check failed!\n" + checkQuery.lastError().text());
        return;
    }
    if (checkQuery.next())
    {
        QMessageBox::critical(this, "Error", "Username already exists!");
        return;
    }
    // 插入新用户
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    if (insertQuery.exec())
    {
        QMessageBox::information(this, "成功", "注册成功！");
        this->accept();
    }
    else
    {
        QMessageBox::critical(this, "Database Error", "Failed to sign up!\n" + insertQuery.lastError().text());
    }
}

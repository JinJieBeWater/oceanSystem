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
    connect(ui->pushButton, &QPushButton::clicked, this, &signupdialog::onSignUpClicked);
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
        QMessageBox::critical(this, "错误", "用户名或密码不能为空！");
        return;
    }
    if (username.length() < 3 || password.length() < 6)
    {
        QMessageBox::critical(this, "错误", "用户名至少3位，密码至少6位！");
        return;
    }
    if (username.length() > 20 || password.length() > 20)
    {
        QMessageBox::critical(this, "错误", "用户名和密码不能超过20位！");
        return;
    }
    if (username.contains(" ") || password.contains(" "))
    {
        QMessageBox::critical(this, "错误", "用户名和密码不能包含空格！");
        return;
    }
    if (username.contains(";") || password.contains(";"))
    {
        QMessageBox::critical(this, "错误", "用户名和密码不能包含分号！");
        return;
    }
    if (!db.isOpen())
    {
        QMessageBox::critical(this, "数据库错误", "数据库未打开！");
        return;
    }
    // 检查用户名是否已存在
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id FROM users WHERE username = :username");
    checkQuery.bindValue(":username", username);
    if (!checkQuery.exec())
    {
        QMessageBox::critical(this, "数据库错误", "检查失败！\n" + checkQuery.lastError().text());
        return;
    }
    if (checkQuery.next())
    {
        QMessageBox::critical(this, "错误", "用户名已存在！");
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
        QMessageBox::critical(this, "数据库错误", "注册失败！\n" + insertQuery.lastError().text());
    }
}

#include "usermanagewin.h"
#include "ui_usermanagewin.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTableWidgetItem>

#include <QDebug>

usermanagewin::usermanagewin(QWidget *parent) : QMainWindow(parent),
                                                ui(new Ui::usermanagewin)
{
    ui->setupUi(this);
    // 检查是否已有默认连接
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("D:/University/yue_inlay/oceanSystem.db");
    }

    if (!db.open())
    {
        QMessageBox::critical(this, "Database Error",
                              "Failed to open the database!\n" + db.lastError().text());
        qDebug() << "Database error: " << db.lastError().text();
    }
    else
    {
        qDebug() << "Database opened successfully.";
    }
    refreshTable();
}

usermanagewin::~usermanagewin()
{
    delete ui;
}

void usermanagewin::on_btnSearchUserName_clicked()
{
    QString username = ui->editSearchUserName->text().trimmed();
    refreshTable(username);
}

void usermanagewin::refreshTable(const QString &usernameFilter)
{
    ui->tableWidget->setRowCount(0);
    if (!db.isOpen())
    {
        QMessageBox::critical(this, "数据库错误", "数据库未打开！");
        return;
    }
    QSqlQuery query(db);
    QString sql = "SELECT id, username, password FROM users";
    if (!usernameFilter.isEmpty())
    {
        sql += " WHERE username LIKE ?";
    }
    sql += " ORDER BY id ASC";
    query.prepare(sql);
    if (!usernameFilter.isEmpty())
    {
        query.addBindValue("%" + usernameFilter + "%");
    }
    if (!query.exec())
    {
        QMessageBox::critical(this, "查询失败", query.lastError().text());
        return;
    }
    QIcon icon("D:/University/yue_inlay/qt/oceanSystem/resources/user.png");
    int row = 0;
    ui->tableWidget->setIconSize(QSize(32, 32));
    while (query.next())
    {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(icon, query.value(0).toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
}

void usermanagewin::on_btnCreateConfirm_clicked()
{
    QString username = ui->editCreateUserName->text().trimmed();
    QString password = ui->editCreatePassword->text();
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空！");
        return;
    }
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id FROM users WHERE username = ?");
    checkQuery.addBindValue(username);
    if (!checkQuery.exec() || checkQuery.next())
    {
        QMessageBox::warning(this, "提示", "用户名已存在！");
        return;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    if (query.exec())
    {
        QMessageBox::information(this, "成功", "用户创建成功！");
        ui->editCreateUserName->clear();
        ui->editCreatePassword->clear();
        refreshTable();
    }
    else
    {
        QMessageBox::critical(this, "失败", "创建用户失败！\n" + query.lastError().text());
    }
}

void usermanagewin::on_btnEditConfirm_clicked()
{
    QString id = ui->editEditUserId->text().trimmed();
    QString newUsername = ui->editEditUserName->text().trimmed();
    QString newPassword = ui->editEditPassword->text();
    if (id.isEmpty())
    {
        QMessageBox::warning(this, "提示", "用户ID不能为空！");
        return;
    }
    if (newUsername.isEmpty() && newPassword.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请至少填写新用户名或新密码！");
        return;
    }
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id FROM users WHERE id = ?");
    checkQuery.addBindValue(id);
    if (!checkQuery.exec() || !checkQuery.next())
    {
        QMessageBox::warning(this, "提示", "用户ID不存在！");
        return;
    }
    QString sql = "UPDATE users SET ";
    QList<QVariant> binds;
    if (!newUsername.isEmpty())
    {
        sql += "username = ?";
        binds << newUsername;
    }
    if (!newPassword.isEmpty())
    {
        if (!binds.isEmpty())
            sql += ", ";
        sql += "password = ?";
        binds << newPassword;
    }
    sql += " WHERE id = ?";
    binds << id;
    QSqlQuery query(db);
    query.prepare(sql);
    for (const QVariant &v : binds)
        query.addBindValue(v);
    if (query.exec())
    {
        QMessageBox::information(this, "成功", "用户信息已更新！");
        ui->editEditUserId->clear();
        ui->editEditUserName->clear();
        ui->editEditPassword->clear();
        refreshTable();
    }
    else
    {
        QMessageBox::critical(this, "失败", "更新用户信息失败！\n" + query.lastError().text());
    }
}

void usermanagewin::on_btnDeleteConfirm_clicked()
{
    QString id = ui->editDeleteUserId->text().trimmed();
    QString username = ui->editDeleteUserName->text().trimmed();
    if (id.isEmpty() && username.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请至少输入用户ID或用户名！");
        return;
    }
    QSqlQuery query(db);
    if (!id.isEmpty())
    {
        query.prepare("DELETE FROM users WHERE id = ?");
        query.addBindValue(id);
    }
    else
    {
        query.prepare("DELETE FROM users WHERE username = ?");
        query.addBindValue(username);
    }
    if (query.exec() && query.numRowsAffected() > 0)
    {
        QMessageBox::information(this, "成功", "用户已删除！");
        ui->editDeleteUserId->clear();
        ui->editDeleteUserName->clear();
        refreshTable();
    }
    else
    {
        QMessageBox::warning(this, "失败", "未找到匹配的用户或删除失败！");
    }
}

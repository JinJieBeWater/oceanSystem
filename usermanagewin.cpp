#include "usermanagewin.h"
#include "ui_usermanagewin.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTableWidgetItem>

#include "Log.h"

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
        db.setDatabaseName("D:/University/Yue inlay/oceanSystem.db");
    }

    if (!db.open())
    {
        QMessageBox::critical(this, "Database Error",
                              "Failed to open the database!\n" + db.lastError().text());
        Log() << "Database error: " << db.lastError().text();
    }
    else
    {
        Log() << "Database opened successfully.";
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
    int row = 0;
    while (query.next())
    {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        row++;
    }
}

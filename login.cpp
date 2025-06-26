#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "mainwin.h"
#include "signupdialog.h"
#include "Log.h"

login::login(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::login)
{
    ui->setupUi(this);
    speech = new QTextToSpeech(this);

    // 初始化数据库
    initializeDatabase();

    mainWin = new mainwin(this);
}

login::~login()
{
    // 关闭数据库连接
    if (db.isOpen())
    {
        db.close();
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    delete ui;
}

void login::initializeDatabase()
{
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
        createTables(); // 确保表存在
    }
}

void login::createTables()
{
    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "password TEXT NOT NULL)"))
    {
        Log() << "Table creation error: " << query.lastError().text();
        QMessageBox::critical(this, "Database Error",
                              "Failed to create tables!\n" + query.lastError().text());
    }
}

void login::on_signInBtn_clicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Database Error", "Database is not open!");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        Log() << "Login query error: " << query.lastError().text()
              << "\nQuery: " << query.lastQuery()
              << "\nBound values: " << query.boundValues();
        QMessageBox::critical(this, "Database Error",
                              "Query failed!\n" + query.lastError().text());
        return;
    }

    if (query.next())
    {
        Log() << "User" << username << "logged in successfully";
        speech->say("Sign In successful!");
        mainWin->show();
        this->hide();
    }
    else
    {
        Log() << "Failed login attempt for user" << username;
        QMessageBox::critical(this, "Error", "Invalid username or password!");
    }
}

void login::on_signUpBtn_clicked()
{
    signupdialog signupDialog(this);
    signupDialog.getDatabase(db); // Pass the database connection to the signup dialog
    signupDialog.exec();
}
